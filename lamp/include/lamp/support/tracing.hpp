/*
 * (c) 2021 Henrich Lauko <xlauko@mail.muni.cz>
 * (c) 2020 Petr Ročkai <code@fixp.eu>
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

#pragma once

#include <runtime/stream.hpp>
#include <runtime/config.hpp>
#include <lava/support/tristate.hpp>

#include <array>

namespace __lamp
{
    using namespace std::literals;

    template< typename domain >
    struct tracing_stream_base
    {
        using domain_ref = const domain&;

        struct traced_result
        {
            traced_result( std::string_view op, domain_ref r ) : op( op ), result( r ) {}

            std::string_view op;
            domain_ref result;
        };

        struct traced_cast : traced_result
        {
            using bw = typename domain::bw;
            
            traced_cast( std::string_view op, domain_ref r, domain_ref a, bw b )
                : traced_result( op, r ), arg( a ), bitwidth( b ) {}

            domain_ref arg;
            bw bitwidth;
        };

        struct traced_unary : traced_result
        {
            traced_unary( std::string_view op, domain_ref r, domain_ref a )
                : traced_result( op, r ), arg( a ) {}

            domain_ref arg;
        };
        
        struct traced_binary : traced_result
        {
            traced_binary( std::string_view op, domain_ref r, domain_ref a, domain_ref b )
                : traced_result( op, r ), left( a ), right( b ) {}

            domain_ref left, right;
        };

        struct traced_assume
        {
            traced_assume( domain_ref a, bool e ) : arg( a ), expected( e ) {}
    
            domain_ref arg;
            bool expected;
        };
    };

    template< typename domain, typename stream >
    struct tracing_domain : domain
    {
        using domain::domain;

        using bw = typename domain::bw;
        
        using self = domain;
        using sref = const domain &;

        using traced_result = typename stream::traced_result;
        using traced_assume = typename stream::traced_assume;
        using traced_cast   = typename stream::traced_cast;
        using traced_unary  = typename stream::traced_unary;
        using traced_binary = typename stream::traced_binary;

        template< typename op_t, typename ...args_t >
        static domain trace( std::string_view fn, op_t op )
        {
            auto s = stream();
            auto res = op();
            s << traced_result( fn, res ) << "\n";
            return std::move( res );
        }
        
        template< typename op_t, typename ...args_t >
        static domain trace( std::string_view fn, op_t op, sref a )
        {
            auto s = stream();
            auto res = op( a );
            s << traced_unary( fn, res, a ) << "\n";
            return std::move( res );
        }

        template< typename op_t, typename ...args_t >
        static domain trace( std::string_view fn, op_t op, sref a, sref b )
        {
            auto s = stream();
            auto res = op( a, b );
            s << traced_binary( fn, res, a, b ) << "\n";
            return std::move( res );
        }

        template< typename op_t >
        static domain trace( std::string_view fn, op_t op, sref a, bw b )
        {
            auto s = stream();
            auto res = op( a, b );
            s << traced_cast( fn, res, a, b ) << "\n";
            return std::move( res );
        }

        static void trace( std::string_view, decltype( domain::assume ) op, self &a, bool expected )
        {
            auto s = stream();
            op( a, expected );
            s << traced_assume( a, expected ) << "\n";
        }
        
        #define TRACE(...) trace(__FUNCTION__, __VA_ARGS__);
        
        template< typename val_t >
        static self lift( const val_t &val ) { return TRACE( domain::lift, val ); }

        template< typename val_t >
        static self any() { return TRACE( domain::template any< val_t > ); }

        static void assume( self &a, bool expected ) { TRACE( domain::assume, a, expected ); }

        /* arithmetic operations */
        static self op_add ( sref a, sref b ) { return TRACE( domain::op_add, a, b ); }
        static self op_fadd( sref a, sref b ) { return TRACE( domain::op_fadd, a, b ); }
        static self op_sub ( sref a, sref b ) { return TRACE( domain::op_sub, a, b ); }
        static self op_fsub( sref a, sref b ) { return TRACE( domain::op_fsub, a, b ); }
        static self op_mul ( sref a, sref b ) { return TRACE( domain::op_mul, a, b ); }
        static self op_fmul( sref a, sref b ) { return TRACE( domain::op_fmul, a, b ); }
        static self op_udiv( sref a, sref b ) { return TRACE( domain::op_udiv, a, b ); }
        static self op_sdiv( sref a, sref b ) { return TRACE( domain::op_sdiv, a, b ); }
        static self op_fdiv( sref a, sref b ) { return TRACE( domain::op_fdiv, a, b ); }
        static self op_urem( sref a, sref b ) { return TRACE( domain::op_urem, a, b ); }
        static self op_srem( sref a, sref b ) { return TRACE( domain::op_srem, a, b ); }
        static self op_frem( sref a, sref b ) { return TRACE( domain::op_frem, a, b ); }

        static self op_fneg( sref a ) { return TRACE( domain::op_fneg, a ); }

        // /* bitwise operations */
        static self op_shl ( sref a, sref b ) { return TRACE( domain::op_shl, a, b ); }
        static self op_lshr( sref a, sref b ) { return TRACE( domain::op_lshr, a, b ); }
        static self op_ashr( sref a, sref b ) { return TRACE( domain::op_ashr, a, b ); }
        static self op_and ( sref a, sref b ) { return TRACE( domain::op_and, a, b ); }
        static self op_or  ( sref a, sref b ) { return TRACE( domain::op_or, a, b ); }
        static self op_xor ( sref a, sref b ) { return TRACE( domain::op_xor, a, b ); }

        // /* comparison operations */
        static self op_foeq( sref a, sref b ) { return TRACE( domain::op_foeq, a, b ); }
        static self op_fogt( sref a, sref b ) { return TRACE( domain::op_fogt, a, b ); }
        static self op_foge( sref a, sref b ) { return TRACE( domain::op_foge, a, b ); }
        static self op_folt( sref a, sref b ) { return TRACE( domain::op_folt, a, b ); }
        static self op_fole( sref a, sref b ) { return TRACE( domain::op_fole, a, b ); }
        static self op_fone( sref a, sref b ) { return TRACE( domain::op_fone, a, b ); }
        static self op_ford( sref a, sref b ) { return TRACE( domain::op_ford, a, b ); }
        static self op_funo( sref a, sref b ) { return TRACE( domain::op_funo, a, b ); }
        static self op_fueq( sref a, sref b ) { return TRACE( domain::op_fueq, a, b ); }
        static self op_fugt( sref a, sref b ) { return TRACE( domain::op_fugt, a, b ); }
        static self op_fuge( sref a, sref b ) { return TRACE( domain::op_fuge, a, b ); }
        static self op_fult( sref a, sref b ) { return TRACE( domain::op_fult, a, b ); }
        static self op_fule( sref a, sref b ) { return TRACE( domain::op_fule, a, b ); }
        static self op_fune( sref a, sref b ) { return TRACE( domain::op_fune, a, b ); }

        static self op_eq ( sref a, sref b ) { return TRACE( domain::op_eq, a, b ); }
        static self op_ne ( sref a, sref b ) { return TRACE( domain::op_ne, a, b ); }
        static self op_ugt( sref a, sref b ) { return TRACE( domain::op_ugt, a, b ); }
        static self op_uge( sref a, sref b ) { return TRACE( domain::op_uge, a, b ); }
        static self op_ult( sref a, sref b ) { return TRACE( domain::op_ult, a, b ); }
        static self op_ule( sref a, sref b ) { return TRACE( domain::op_ule, a, b ); }
        static self op_sgt( sref a, sref b ) { return TRACE( domain::op_sgt, a, b ); }
        static self op_sge( sref a, sref b ) { return TRACE( domain::op_sge, a, b ); }
        static self op_slt( sref a, sref b ) { return TRACE( domain::op_slt, a, b ); }
        static self op_sle( sref a, sref b ) { return TRACE( domain::op_sle, a, b ); }

        static self op_ffalse( sref a, sref b ) { return TRACE( domain::op_ffalse, a, b ); }
        static self op_ftrue( sref a, sref b )  { return TRACE( domain::op_ftrue, a, b ); }

        static self op_fpext   ( sref a, bw b ) { return TRACE( domain::op_fpext, a, b ); }
        static self op_fptosi  ( sref a, bw b ) { return TRACE( domain::op_fptosi, a, b ); }
        static self op_fptoui  ( sref a, bw b ) { return TRACE( domain::op_fptoui, a, b ); }
        static self op_fptrunc ( sref a, bw b ) { return TRACE( domain::op_fptrunc, a, b ); }
        static self op_inttoptr( sref a, bw b ) { return TRACE( domain::op_inttoptr, a, b ); }
        static self op_ptrtoint( sref a, bw b ) { return TRACE( domain::op_ptrtoint, a, b ); }
        static self op_sext    ( sref a, bw b ) { return TRACE( domain::op_sext, a, b ); }
        static self op_sitofp  ( sref a, bw b ) { return TRACE( domain::op_sitofp, a, b ); }
        static self op_trunc   ( sref a, bw b ) { return TRACE( domain::op_trunc, a, b ); }
        static self op_uitofp  ( sref a, bw b ) { return TRACE( domain::op_uitofp, a, b ); }
        static self op_zext    ( sref a, bw b ) { return TRACE( domain::op_zext, a, b ); }
        static self op_zfit    ( sref a, bw b ) { return TRACE( domain::op_zfit, a, b ); }
    };

    template< typename domain, typename outstream >
    struct simple_stream : tracing_stream_base< domain >, outstream
    {
        using base = tracing_stream_base< domain >;

        using traced_result = typename base::traced_result;
        using traced_assume = typename base::traced_assume;
        using traced_cast   = typename base::traced_cast;
        using traced_unary  = typename base::traced_unary;
        using traced_binary = typename base::traced_binary;
        
        outstream& underlying() { return *static_cast< outstream* >( this ); }

        simple_stream& operator<<(std::string_view str) noexcept
        {
            underlying() << str;
            return *this;
        }

        simple_stream& operator<<(const traced_result &r) noexcept
        {
            underlying() << r.op << " ➞  " << r.result;
            return *this;
        }
        
        simple_stream& operator<<(const traced_assume &a) noexcept
        {
            underlying() << "assume: " << (a.expected ? "" : "not ") << a.arg;
            return *this;
        }

        simple_stream& operator<<(const traced_cast &c) noexcept
        {
            underlying() << c.op << " " << c.arg << " ➞  " << c.result;
            return *this;
        }

        simple_stream& operator<<(const traced_unary &u) noexcept
        {
            underlying() << u.op << " " << u.arg << " ➞  " << u.result;
            return *this;
        }
        
        simple_stream& operator<<(const traced_binary &b) noexcept
        {
            underlying() << b.op << " " << b.left << " " << b.right << " ➞  " << b.result;
            return *this;
        }
    };

    template< typename domain, typename outstream >
    struct json_stream : tracing_stream_base< domain >, outstream
    {
        using base = tracing_stream_base< domain >;

        using domain_ref = typename base::domain_ref;

        using traced_result = typename base::traced_result;
        using traced_assume = typename base::traced_assume;
        using traced_cast   = typename base::traced_cast;
        using traced_unary  = typename base::traced_unary;
        using traced_binary = typename base::traced_binary;

        template< typename value_t >
        struct keyvalue
        {
            keyvalue(std::string_view k, const value_t &v ) : key( k ), value( v ) {}
            std::string_view key;
            const value_t &value;

            template< typename type >
            struct asstring
            {
                asstring( const type &v ) : value( v ) {}
                const type &value;

                template< typename stream >
                friend stream& operator<<( stream &os, const asstring &s )
                {
                    return os << "\"" << s.value << "\"";
                }
            };

            template< typename type > asstring( const type& ) -> asstring< type >;

            template< typename stream >
            friend stream& operator<<( stream &os, const keyvalue &kv )
            {
                os << asstring( kv.key ) << ": ";
                if constexpr ( std::is_same_v< value_t, domain > ) {
                    return os << asstring( kv.value );
                } else if constexpr ( std::is_same_v< value_t, std::string_view > ) {
                    return os << asstring( kv.value );
                } else if constexpr ( std::is_integral_v< value_t > ) {
                    std::array< char, 65 > buff{};
                    std::snprintf( buff.data(), buff.size(), "%d", kv.value );
                    return os << std::string_view( buff.data() );
                } else {
                    return os << kv.value;
                }
            }
        };
        
        template< typename value_t >
        keyvalue( std::string_view, const value_t& ) -> keyvalue< value_t >;


        struct value_with_address
        {
            using address = std::array< char, 16 >;

            value_with_address( std::string_view n, domain_ref v )
                : name( n ), value( v )
            {
                std::snprintf(addr.data(), addr.size(), "%p", v.unsafe_ptr());
            }

            template< typename stream >
            friend stream& operator<<( stream &os, const value_with_address &v )
            {
                return os << keyvalue( v.name, std::tuple( 
                    keyvalue( "value", v.value ),
                    keyvalue( "addr", std::string_view( v.addr.data() ) )
                ));
            }

            std::string_view name;
            domain_ref value;
            address addr;
        };


        template< typename F, typename ...types >
        F for_all( F fn, types &&... values )
        {
            ( fn( std::forward< types >( values ) ), ... );
            return std::move( fn );
        }

        template< typename F, typename ...types, std::size_t ...indices >
        F for_all_indices( F fn, std::tuple< types... > const & t, std::index_sequence< indices... > )
        {
            return for_all( std::move( fn ), std::get< indices >(t)... );
        }

        template< typename first, typename ...rest > // non-nullary tuples only
        json_stream& operator<<( std::tuple< first, rest... > const &t )
        {
            self() << "{";
            for_all_indices( [&]( auto const &value ) { 
                self() << value << ", "; }, 
                t, std::index_sequence_for< rest... >{}
            );
            self() << std::get< sizeof...(rest) >(t) << "}";
            return self();
        }


        outstream& underlying() { return *static_cast< outstream* >( this ); }
        json_stream& self() { return *this; }

        json_stream& operator<<(std::string_view str) noexcept
        {
            underlying() << str;
            return self();
        }

        json_stream& operator<<(const traced_result &r) noexcept
        {
            return self() << std::tuple(
                keyvalue( "operation", r.op ),
                value_with_address( "result", r.result )
            );
        }
        
        json_stream& operator<<(const traced_assume &a) noexcept
        {
            return self() << std::tuple(
                keyvalue( "type", "assume"sv ),
                value_with_address( "arg", a.arg ),
                keyvalue( "expected", (a.expected ? "true" : "false") )
            );
        }

        json_stream& operator<<(const traced_cast &c) noexcept
        {
            return self() << std::tuple(
                keyvalue( "operation", c.op ),
                keyvalue( "type", "cast"sv ),
                value_with_address( "result", c.result ),
                value_with_address( "arg", c.arg ),
                keyvalue( "bitwidth", c.bitwidth )
            );
        }

        json_stream& operator<<(const traced_unary &u) noexcept
        {
            return self() << std::tuple(
                keyvalue( "operation", u.op ),
                keyvalue( "type", "unary"sv ),
                value_with_address( "result", u.result ),
                value_with_address( "arg", u.arg )
            );;
        }
        
        json_stream& operator<<(const traced_binary &b) noexcept
        {
            return self() << std::tuple(
                keyvalue( "operation", b.op ),
                keyvalue( "type", "binary"sv ),
                value_with_address( "result", b.result ),
                value_with_address( "left", b.left ),
                value_with_address( "right", b.right )
            );;
        }
    };

    using file_stream = __lart::rt::file_stream;

    struct configured_stream : file_stream
    {
        file_stream& underlying() { return *static_cast< file_stream* >( this ); }
        
        configured_stream()
        {
            auto file = __lart::rt::config.trace_file;
            if ( file )
                this->_file = file;
        }
        
        configured_stream& operator<<(std::string_view str) noexcept
        {
            underlying() << str;
            return *this;
        }
    };

    template< typename domain >
    using tracing = tracing_domain< domain, simple_stream< domain, configured_stream > >;

    template< typename domain >
    using jsontracing = tracing_domain< domain, json_stream< domain, configured_stream > >;

} // namespace __lamp
