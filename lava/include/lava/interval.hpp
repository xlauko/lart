
#include <lava/support/base.hpp>
#include <lava/support/scalar.hpp>
#include <lava/support/reference.hpp>
#include <lava/constant.hpp>

#include <lava/support/interval.hpp>

#include <lamp/support/semilattice.hpp>

namespace __lava
{
    using bound_t = sup::bound< int64_t >;
    using interval_t = sup::interval< bound_t >;

    template< template< typename > typename storage >
    struct interval : storage< interval_t >
                    , domain_mixin< interval< storage > >
    {
        using base = storage< interval_t >;
        using mixin = domain_mixin< interval >;
        using constant_type = constant< __lamp::tagged_storage >;

        using bw = typename mixin::bw;
        using base::base;

        using bound = bound_t;
        using interval_value = interval_t;
        using bound_type = bound::value_t;
        using interval_storage = storage< interval_t >;

        using si = scalar_domain_ref< interval >;
        using iv = interval;
        using ir = const interval &;

        using ref = domain_ref< interval >;

        constexpr static bound choose_bound = 200;


        interval( sup::interval< bound > i ) : interval_storage( i )
        {
            if ( is_bottom() )
                __lart_cancel();
        }

        interval( bound a, bound b ) : interval( sup::interval( a, b ) ) {}

        template< typename type >
        interval( type v )
            : interval( v, v )
        {
            static_assert( sizeof( type ) <= sizeof( bound ) );
        }

        interval( tristate t )  : interval_storage( t ) {}
        interval( bool t )  : interval_storage( t ) {}
        interval clone() const {
            auto const &self = static_cast< const interval& > ( *this );
            return interval( self->low, self->high );
        }

        const bound& low() const { return this->get().low; }
        const bound& high() const { return this->get().high; }

        static bound plus_infinity() { return bound::plus_infinity(); }
        static bound minus_infinity() { return bound::minus_infinity(); }

        bool is_infinite() const { return low() == minus_infinity() || high() == plus_infinity(); }
        bool is_finite() const   { return !is_infinite(); }

        bool is_top() const    { return this->get().is_top(); }
        bool is_bottom() const { return this->get().is_bottom(); }

        void normalize()
        {
            if ( is_bottom() )
            {
                this->get() = sup::interval< bound >( 0, -1 );
            }
        }

        static interval top() { return { minus_infinity(), plus_infinity() }; }
        static interval bottom() { return interval( 0, -1 ); }

        constexpr bool includes( ir o ) { return low() <= o->low && o->high <= high(); }
        constexpr bool excludes( ir o ) { return o->high < low() || high() < o->low; }

        static iv lift( const constant_type& con ) { return constant_type::lift_to< interval >( con ); }

        template< typename type > static auto lift( type v )
            -> std::enable_if_t< std::is_integral_v< type >, interval >
        {
            return interval( v );
        }

        template< typename type > static auto lift( type /* v */ )
            -> std::enable_if_t< !std::is_integral_v< type >, interval >
        {
            return mixin::fail();
        }

        static constant_type lower( ir i )
        {
            return constant_type::lift(
                bound_type( i.low() ) + __lart_choose( i.get().size() )
            );
        }

        template< typename type > static iv any()
        {
            return { minus_infinity(), plus_infinity() };
        }

        void intersect( const interval_value &other )
        {
            this->get().intersect( other );

            if ( this->is_bottom() ) {
                __lart_cancel();
            }
        }

        static void assume( interval &i, bool constraint )
        {
            i.intersect( interval_value( constraint ) );
        }

        static tristate to_tristate( ir i )
        {
            return static_cast< __lava::tristate >( i.get() );
        }

        /* lattice operations */
        static iv op_join( ir l, ir r ) { return join( l.get(), r.get() ); }
        static iv op_meet( ir l, ir r ) { return meet( l.get(), r.get() ); }

        static iv op_add ( ir a, ir b ) { return a.get() + b.get(); }
        static iv op_sub ( ir a, ir b ) { return a.get() - b.get(); }
        static iv op_mul ( ir a, ir b ) { return a.get() * b.get(); }
        static iv op_sdiv( ir a, ir b ) { return a.get() / b.get(); }
        static iv op_udiv( ir a, ir b ) { return a.get() / b.get(); } // FIXME
        static iv op_srem( ir a, ir b ) { return a.get() % b.get(); }
        static iv op_urem( ir a, ir b ) { return a.get() % b.get(); } // FIXME

        // unbounded interval domain ignores bitwidth
        static iv op_sext(  ir i, bw ) { return i.clone(); }
        static iv op_trunc( ir i, bw ) { return i.clone(); }
        static iv op_zext(  ir i, bw ) { return i.clone(); }
        static iv op_zfit(  ir i, bw ) { return i.clone(); }

        static iv op_eq ( ir a, ir b ) { return a.get() == b.get(); }
        static iv op_ne ( ir a, ir b ) { return a.get() != b.get(); }
        static iv op_ugt( ir a, ir b ) { return a.get() > b.get();  } // FIXME
        static iv op_uge( ir a, ir b ) { return a.get() >= b.get(); } // FIXME
        static iv op_ult( ir a, ir b ) { return a.get() < b.get();  } // FIXME
        static iv op_ule( ir a, ir b ) { return a.get() <= b.get(); } // FIXME
        static iv op_sgt( ir a, ir b ) { return a.get() >  b.get(); }
        static iv op_sge( ir a, ir b ) { return a.get() >= b.get(); }
        static iv op_slt( ir a, ir b ) { return a.get() <  b.get(); }
        static iv op_sle( ir a, ir b ) { return a.get() <= b.get(); }

        static iv op_shl ( ir a, ir b ) { return a.get() << b.get(); }
        static iv op_ashr( ir a, ir b ) { return a.get() >> b.get(); } //FIXME
        static iv op_lshr( ir a, ir b ) { return a.get() >> b.get(); }
        static iv op_and ( ir /* a */, ir /* b */ ) { return { plus_infinity(), minus_infinity() }; } //FIXME
        static iv op_or  ( ir /* a */, ir /* b */ ) { return { plus_infinity(), minus_infinity() }; } //FIXME
        static iv op_xor ( ir /* a */, ir /* b */ ) { return { plus_infinity(), minus_infinity() }; } //FIXME

        static void validity_check( interval &i )
        {
            if ( i.is_bottom() )
                __lart_cancel();
        }

        static void exclude( interval &i, bound b )
        {
            if ( i->low == b )
                i->low.increment();
            if ( i->high == b)
                i->high.decrement();
            validity_check( i );
        }

        static void badd_( ir r, interval &a, interval &b )
        {
            auto r_new = a.get() + b.get();
            if ( r_new == r.get() ) return;

            auto diff_low = r.low() - r_new.low;
            auto diff_high = r_new.high - r.high();

            // lower bound correction
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( choose_bound ) ) {
                auto i = __lart_choose( int(bound_type( diff_low + 1 )) );
                a->meet_low( a.low() + i );
                b->meet_low( b.low() + diff_low - i );
            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( choose_bound ) ) {
                auto i = __lart_choose( int(bound_type( diff_high + 1 )) );
                a->meet_high( a.high() - i );
                b->meet_high( b.high() - diff_high + i );
            }
        }

        static void bsub_( ir r, interval &a, interval &b )
        {
            auto r_new = a.get() - b.get();
            if ( r_new == r.get() ) return;

            auto diff_low = r.low() - r_new.low;
            auto diff_high = r_new.high - r.high();

            // lower bound correction
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( choose_bound ) ) {
                auto i = __lart_choose( int(bound_type( diff_low + 1 )) );
                a->meet_low( a.low() + i );
                b->meet_high( b.high() - diff_low + i );
            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( choose_bound ) ) {
                auto i = __lart_choose( int(bound_type( diff_high + 1 )) );
                a->meet_high( a.high() - i );
                b->meet_low( b.low() + diff_high - i );
            }
        }

        //div with round up
        static bound div_up( bound a, bound b )
        {
            return ( a / b ) + bound( a % b != bound( 0 ) );
        }

        static void bmul_( ir r, interval &a, interval &b )
        {
            auto r_new = a.get() * b.get();
            if ( r->includes( r_new ) )
                return;

            auto diff_low = r.low() - r_new.low;
            auto diff_high = r_new.high - r.high();

            // lower bound correction
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( choose_bound ) ) {
                auto new_high = div_up( r.low(), b.low() );
                auto new_low = div_up( r.low(), b.high() );
                a->meet_low( new_low );
                auto i = __lart_choose( int(bound_type( new_high - new_low + 1 )) );
                a->meet_low( new_high - i );
                b->meet_low( div_up( r.low(), ( new_high - i ) ) );
                if ( a.is_bottom() || b.is_bottom() )
                    __lart_cancel();

            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( choose_bound ) ) {
                auto new_low = std::max( r.high() / b.high(), a.low() );
                auto i = __lart_choose( int(bound_type( a.high() - new_low + 1 )) );
                a->meet_high( new_low + i );
                b->meet_high( r.high() / ( new_low + i ) );
                if ( a.is_bottom() || b.is_bottom() )
                    __lart_cancel();
            }
        }


        static void bop_add( ir r, ref a, ref b )
        {
            a.intersect( r.get() - b.get() );
            b.intersect( r.get() - a.get() );
            badd_( r, a, b );
        }

        static void bop_sub( ir r, ref a, ref b )
        {
            a.intersect( r.get() + b.get() );
            b.intersect( a.get() - r.get() );
            bsub_( r, a, b );
        }

        static void bop_mul( ir r, ref a, ref b )
        {
            //__dios_fault(_VM_F_NotImplemented, "KoK");
            /* If either <a> or <b> are infinite intervals than <r>
             * will be infinite interval as well. In that case one of:
             * <r / a> or <r / b> wont be defined. */
            if ( a.is_infinite() || b.is_infinite() )
                return
            a.intersect( r.get() / b.get() );
            b.intersect( r.get() / a.get() );
            bmul_( r, a, b );
        }

        static void bop_sdiv( ir r, ref a, ref b )
        {
            a.intersect( b.get() * r.get() );
            if ( a.is_finite() && r.is_finite() )
                b.intersect( a.get() / r.get() );
        }

        static void bop_udiv( ir r, ir a, ir b ) { bop_sdiv( r, a, b ); }

        static void bop_join( ir r, ref a, ref b )
        {
            a.intersect( r.get() );
            b.intersect( r.get() );
        }

        static void beq_impl( interval &a, interval &b )
        {
            a.intersect( b.get() );
            b.intersect( a.get() );
            auto interval_size = a.high() - a.low();
            if ( interval_size > bound( 0 ) && interval_size <= bound( choose_bound ) ) {
                auto i = __lart_choose( int(bound_type( interval_size )) );
                a.intersect( { a.low() + i, a.low() + i } );
                b.intersect( { b.low() + i, b.low() + i } );
            }
        }

        static void bne_impl( interval &a, interval &b )
        {

            auto intersection = meet( a.get(), b.get() );
            if ( !intersection.empty() ) {
                if ( a->constant() && ( a->low == b->low || a->low == b->high ) )
                    return exclude( b, a->low );
                if ( b->constant() && ( b->low == a->low || b->low == a->high ) )
                    return exclude( a, b->low );
                __lart_choose( 2 ) ? bgt_( a, b ) : bgt_( b, a );
            }
        }

        static void bgt_( interval &a, interval &b, bool strict = true )
        {
            a->meet_low( b.low() );
            if ( strict )
                exclude( a, b.low() );
            b->meet_high( a.high() );
            if ( strict )
                exclude( b, a.high() );
            auto intersection = meet( a.get(), b.get() );
            if ( intersection.empty() )
                return;

            auto size = intersection.size();
            if ( choose_bound < bound_type( size ) )
                return;

            auto delim = __lart_choose( strict ? int(size + 1) : int(size) ) + intersection.low;
            a.intersect( { delim, a.high() } );
            b.intersect( { b.low(), strict ? delim - 1 : delim } );
        }

        static void beq( ir r, ref a, ref b, bool negated = false )
        {
            if ( r.get() == !negated ) beq_impl( a, b );
            if ( r.get() ==  negated ) bne_impl( a, b );
        }

        static void bgt( ir r, ref a, ref b, bool negated = false )
        {
            if ( r.get() == !negated ) bgt_( a, b );
            if ( r.get() ==  negated ) bgt_( b, a, false /* non-strict */ );
        }

        static void bop_eq( ir r, ir a, ir b ) { beq( r, a, b ); }
        static void bop_ne( ir r, ir a, ir b ) { beq( r, a, b, true /* negated */ ); }

        static void bop_sgt( ir r, ir a, ir b ) { bgt( r, a, b ); }
        static void bop_slt( ir r, ir a, ir b ) { bgt( r, b, a ); }
        static void bop_sge( ir r, ir a, ir b ) { bgt( r, b, a, true /* negated */ ); }
        static void bop_sle( ir r, ir a, ir b ) { bgt( r, a, b, true /* negated */ ); }

        static void bop_ugt( ir r, ir a, ir b ) { bop_sgt( r, a, b ); }
        static void bop_uge( ir r, ir a, ir b ) { bop_sge( r, a, b ); }
        static void bop_ult( ir r, ir a, ir b ) { bop_slt( r, a, b ); }
        static void bop_ule( ir r, ir a, ir b ) { bop_sle( r, a, b ); }

        static void bop_zfit( ir r, ref a )
        {
            a.get() = r.get(); // TODO
        }

        static void dump( ir i )
        {
            printf( "[" );
            if ( i->low == minus_infinity() )
                printf( "-∞" );
            else if ( i->low == plus_infinity() )
                printf( "∞" );
            else
                printf( "%ld", static_cast<long>(i->low) );

            printf( " ," );

            if ( i->high == minus_infinity() )
                printf( "-∞" );
            else if ( i->high == plus_infinity() )
                printf( "∞" );
            else
                printf( "%ld", static_cast<long>(i->high) );

            printf( " ] \n" );
        }

        static std::string trace( ir i )
        {
            /*
            std::stringstream ss;
            ss << "[";
            if ( i->low == minus_infinity() )
                ss << "-∞";
            else if ( i->low == plus_infinity() )
                ss << "∞";
            else
                ss << i->low;

            ss << " ,";

            if ( i->high() == minus_infinity() )
                 ss << "-∞";
            else if ( i->high() == plus_infinity() )
                 ss << "∞";
            else
                 ss << i->high();

            return ss.str();
            */
            return "N/A";
        }

        template< typename stream >
        friend stream& operator<<( stream &os, ir i )
        {
            os << "[";
            if ( i->low == minus_infinity() )
                os << "-∞";
            else if ( i->low == plus_infinity() )
                os << "∞";
            else
                os << i->low;

            os << " ,";

            if ( i->high == minus_infinity() )
                 os << "-∞";
            else if ( i->high == plus_infinity() )
                 os << "∞";
            else
                 os << i->high;

            os << " ] \n";
            return os;
        }
    };
}
