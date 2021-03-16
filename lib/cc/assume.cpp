/*
 * (c) 2016 Henrich Lauko <xlauko@mail.muni.cz>
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

#include <cc/assume.hpp>

#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace lart::constrain
{
    void bbedge::hide()
    {
        assert( from->getUniqueSuccessor() );
        from->getTerminator()->setSuccessor( 0, from );
    }

    void bbedge::show()
    {
        assert( from->getUniqueSuccessor() );
        from->getTerminator()->setSuccessor( 0, to );
    }

    void replace_phis_incoming_bbs( basicblock *bb, basicblock *oldbb, basicblock *newbb )
    {
        for ( auto& inst : *bb )
            if ( auto phi = llvm::dyn_cast< llvm::PHINode >( &inst ) )
            {
                int bbidx = phi->getBasicBlockIndex( oldbb );
                if ( bbidx >= 0 )
                    phi->setIncomingBlock( static_cast< unsigned >( bbidx ), newbb );
            }
    }

    op::operation assume_edge::assume( assumption ass )
    {
        auto succ = succ_idx();
        llvm::SplitEdge( from, to );
        auto edge = from->getTerminator()->getSuccessor( succ );
        to = edge->getSingleSuccessor();

        // Correct phis after edge splitting
        replace_phis_incoming_bbs( to, from, edge );
        return op::assume( ass.cond, &*edge->getFirstInsertionPt(), ass.expect );
    }

    unsigned assume_edge::succ_idx() const
    {
        auto term = from->getTerminator();

        for ( unsigned i = 0; i < term->getNumSuccessors(); ++i )
            if ( term->getSuccessor( i ) == to )
                return i;

        __builtin_unreachable();
        // ( "BasicBlock 'to' is not a successor of BasicBlock 'from'." );
    }

    generator< op::operation > assume( llvm::BranchInst *br )
    {
        auto cond = br->getCondition();
        auto &ctx = br->getContext();

        assume_edge true_br = { br->getParent(), br->getSuccessor( 0 ) };
        co_yield true_br.assume( { cond, llvm::ConstantInt::getTrue( ctx ) } );

        assume_edge false_br = { br->getParent(), br->getSuccessor( 1 ) };
        co_yield false_br.assume( { cond, llvm::ConstantInt::getFalse( ctx ) } );
    }

} // namespace lart::constrain
