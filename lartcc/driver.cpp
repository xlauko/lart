/*
 * (c) 2020, 2021 Henrich Lauko <xlauko@mail.muni.cz>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <cc/driver.hpp>

#include <cc/assume.hpp>
#include <cc/dfa.hpp>
#include <cc/syntactic.hpp>
#include <cc/logger.hpp>
#include <cc/preprocess.hpp>

#include <cc/backend/native.hpp>

#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/Verifier.h>

#include <sc/erase.hpp>
#include <sc/ranges.hpp>

#include <queue>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <string_view>

namespace lart
{
    namespace sv = sc::views;

    void verify( llvm::Module &module )
    {
        llvm::raw_os_ostream cerr( std::cerr );
        if ( llvm::verifyModule( module, &cerr ) ) {
            cerr.flush(); // otherwise nothing will be displayed
            // module.print(llvm::errs(), nullptr);
            std::exit( EXIT_FAILURE );
        }
    }

    void replace_abstractable_functions( llvm::Module &module )
    {
        std::vector< std::pair< llvm::Function *, llvm::Function * > > abstractable;
        for (auto &fn : module) {
            auto name = fn.getName();
            std::string prefix = "__lamp_lifter_";
            if (name.startswith(prefix)) {
                auto suffix = name.drop_front(prefix.size());
                if (auto concrete = module.getFunction(suffix)) {
                    if (concrete->getType() == fn.getType()) {
                        abstractable.emplace_back( &fn, concrete );
                    } else {
                        spdlog::warn("mismatched type of abstractable function {}", suffix);
                    }
                }
            }
        }

        for ( auto [abs, con] : abstractable ) {
            std::vector< llvm::User* > users( con->user_begin(), con->user_end() );
            for ( auto user : users ) {
                user->replaceUsesOfWith( con, abs );
            }
        }
    }


    struct FixGlobals {

        llvm::Function *getNondet( llvm::Type *t, llvm::Module &m )
        {
            if ( auto it = _nondet_map.find( t ); it != _nondet_map.end() )
                return it->second;
            auto sz = t->isPointerTy() ? 64 : t->getScalarSizeInBits();
            sz = sz < 8 ? 8 : sz;
            return _nondet_map.emplace( t, m.getFunction( "__lamp_any_i"
                                            + std::to_string( sz ) ) )
                                    .first->second;
        }

        void initializeGlobal( llvm::Value *g, llvm::IRBuilder<> &irb, llvm::Module &m )
        {
            auto *t = llvm::cast< llvm::PointerType >( g->getType() )->getElementType();

            if ( auto *comp = llvm::dyn_cast< llvm::StructType >( t ) ) {
                for ( uint64_t i = 0, end = comp->getNumElements(); i < end; ++i )
                    initializeGlobal( irb.CreateStructGEP( comp, g, unsigned(i) ), irb, m );
            }
            else if ( auto *arr = llvm::dyn_cast< llvm::ArrayType >( t ) ) {
                for ( uint64_t i = 0, end = arr->getNumElements(); i < end; ++i )
                    initializeGlobal( irb.CreateConstInBoundsGEP2_64( g, 0, unsigned(i) ), irb, m );
            }
            else {
                auto v = irb.CreateCall( getNondet( t, m ), { } );
                irb.CreateStore( irb.CreateBitOrPointerCast( v, t ), g );
            }
        }

        void run( llvm::Module &m )
        {
            auto &ctx = m.getContext();

            auto *init = llvm::cast< llvm::Function >(
                            m.getOrInsertFunction( "__lart_svc_fixglobals_init",
                                llvm::FunctionType::get( llvm::Type::getVoidTy( ctx ), false ) ).getCallee() );
            auto *initBB = llvm::BasicBlock::Create( ctx, "", init );
            llvm::IRBuilder<> irb( initBB, initBB->getFirstInsertionPt() );

            for ( auto &g : m.globals() ) {
                if ( (g.hasExternalLinkage() || g.hasExternalWeakLinkage())
                    && g.isDeclaration() && !g.isConstant() && !g.getName().startswith( "__md_" ) )
                {
                    g.setLinkage( llvm::GlobalValue::InternalLinkage );
                    initializeGlobal( &g, irb, m );
                    g.setInitializer( llvm::UndefValue::get( g.getType()->getElementType() ) );
                }
            }
            irb.CreateRetVoid();

            auto *mainBB = &*m.getFunction( "main" )->begin();
            irb.SetInsertPoint( mainBB, mainBB->getFirstInsertionPt() );
            irb.CreateCall( init, { } );
        }

    private:
        std::map< llvm::Type *, llvm::Function * > _nondet_map;
    };


    struct DropEmptyDecls {
        void run( llvm::Module &m ) {
            std::vector< llvm::Function * > toDrop;
            long all = 0;
            for ( auto &fn : m ) {
                if ( fn.isDeclaration() ) {
                    ++all;
                    if ( fn.user_empty() )
                        toDrop.push_back( &fn );
                }
            }
            for ( auto f : toDrop )
                f->eraseFromParent();
            if ( toDrop.size() )
                spdlog::info("erased: {} empty declarations out of {}", toDrop.size(), all);
        }
    };



    struct StubDecls {
        void run( llvm::Module &m ) {
            auto &ctx = m.getContext();
            auto ptr = llvm::Type::getInt8PtrTy( ctx );
            auto vty = llvm::Type::getVoidTy( ctx );
            auto fty = llvm::FunctionType::get( vty, { ptr }, false );
            auto problem = m.getOrInsertFunction( "__lart_stub_fault", fty );

            const auto undefstr = "lart.stubs: Function stub called.";
            auto undefInit = llvm::ConstantDataArray::getString( ctx, undefstr );
            auto undef = llvm::cast< llvm::GlobalVariable >(
                    m.getOrInsertGlobal( "lart.stubs.undefined.str", undefInit->getType() ) );
            undef->setConstant( true );
            undef->setInitializer( undefInit );

            auto skip = [] (auto &fn) {
                auto name = fn.getName();

                auto list = {
                    "__lamp", "__lart", "__assert_fail",
                    "abort", "atoi", "clearerr", "close", "ctime", "fclose", "ferror", "fflush", "fgetc",
                    "fopen", "fputs", "fread", "fseek", "getenv", "longjmp", "memcmp", "memmove",
                    "putchar", "qsort", "setbuf", "setjmp", "sscanf", "strchr", "strcmp", "strcat", "strncat"
                    "strcpy", "strcspn", "strerror", "strlen", "strncmp", "strncpy", "strrchr",
                    "strstr", "strtod", "strtok", "strtol", "strtoll", "strtoul", "strtoull",
                    "time",
                    "vsnprintf", "fprintf", "printf", "snprintf", "sprintf",
                    "malloc", "calloc", "realloc", "free",
                    "puts", "memcpy", "memset", "alloca", "memchr"
                    "exit"
                };
                
                for (auto pref : list) {
                    if (name.startswith(pref))
                        return true;
                }
                
                return false;
            };

            long all = 0;
            for ( auto &fn : m ) {
                if ( fn.isDeclaration() && !fn.isIntrinsic() && !skip(fn) ) {
                    spdlog::info("stubbing: {}", fn.getName());

                    ++all;
                    auto bb = llvm::BasicBlock::Create( ctx, "", &fn );
                    llvm::IRBuilder<> irb( bb );
                    auto undefPt = irb.CreateBitCast( undef, ptr );
                    irb.CreateCall( problem, { undefPt } );
                    irb.CreateUnreachable();
                }
            }
            if ( all )
                spdlog::info("stubbed: {} declarations", all);
        }
    };

    inline bool option( std::string_view option, std::string_view msg )
    {
        auto is_set = [] ( auto opt ) { return opt && strcmp( opt, "ON" ) == 0; };
        if ( auto opt = std::getenv( option.data() ); is_set( opt ) ) {
            spdlog::info( "[lart config] {}\n", msg );
            return  true;
        }
        return false;
    }

    bool driver::run()
    {
        spdlog::cfg::load_env_levels();
        spdlog::info("lartcc started");

        FixGlobals fixglobals;
        fixglobals.run(module);

        DropEmptyDecls dropEmpty;
        dropEmpty.run(module);

        preprocessor prep(module);
        for (auto &fn : module) {
            prep.run(fn);
        }

        replace_abstractable_functions(module);

        if ( option("LART_STUB", "lart stub missing functions") ) {
            StubDecls stubDecls;
            stubDecls.run(module);
        }
        
        // propagate abstraction type from annotated roots
        auto types = dfa::analysis::run_on( module );

        // syntactic pass
        std::vector< ir::intrinsic > intrinsics;
        syntactic syn( module, types );
        for ( const auto &op : syn.toprocess() ) {
            if ( auto intr = syn.process( op ) ) {
                intrinsics.push_back( intr.value() );
            }
        }

        // 6. release ?

        // 7. interrupts ?

        // TODO pick backend based on cmd arguments
        auto backend = lart::backend::native( module );
        for ( auto intr : intrinsics ) {
            backend.lower( intr );
        }

        // delete stubs
        if ( auto stub = module.getFunction( "__lamp_stub" ) ) {
            stub->deleteBody();
        }

        spdlog::info("lartcc finished");
        // module.getFunction("main")->print(llvm::errs());        
        verify(module);
        return true;
    }

} // namespace lart
