
#include <lava/support/base.hpp>
#include <lava/support/scalar.hpp>
#include <lava/support/reference.hpp>
#include <lava/constant.hpp>

#include <lava/support/interval.hpp>

//#include <lava/support/interval_mapalloc.hpp>

#include <lamp/support/semilattice.hpp>

#include <sstream>
#include <cstdio>
#include <string_view>
#include <cstdarg>
//#include <map>
//#include <vector>
#include <sys/mman.h>
#include <stdlib.h>

namespace __lava
{
    using bound_t = sup::bound< int64_t >;
    using interval_t = sup::interval< bound_t >;

    struct interval_config_t
    {
        bound_t choose_bound = 0;
    };

    /*
    template< typename Key, typename T >
    using Map = std::map<Key, T, std::less<Key>, MyAllocator<std::pair<const Key, T>>>;
    struct interval_state_t
    {
        //std::map< uint, snapshot_t > snapshots;
        //std::map< void*, interval_t > snapshot;
        //std::vector< interval_t > snapshot;
        Map< void*, interval_t > snapshot;
    };*/

    struct interval_state_t
    {
        void* twin;
        unsigned int line;
        interval_t interval;
    };

    struct interval_states_t
    {
        size_t max_size = 0;
        size_t size = 0;
        interval_state_t *states;
    };

    interval_config_t *__interval_cfg;
    //interval_state_t *__interval_states_array;
    interval_states_t *__interval_states;

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
            return interval( static_cast< int >( v ) );
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

        static iv op_add ( ir a, ir b ) { /*dump(a); dump(b);*/ return a.get() + b.get(); }
        static iv op_sub ( ir a, ir b ) { /*dump(a); dump(b);*/ return a.get() - b.get(); }
        static iv op_mul ( ir a, ir b ) { /*dump(a); dump(b);*/ return a.get() * b.get(); }
        static iv op_sdiv( ir a, ir b ) { return a.get() / b.get(); }
        static iv op_udiv( ir a, ir b ) { return a.get() / b.get(); } // FIXME
        static iv op_srem( ir a, ir b ) { return a.get() % b.get(); }
        static iv op_urem( ir a, ir b ) { return a.get() % b.get(); } // FIXME

        // unbounded interval domain ignores bitwidth
        static iv op_sext(  ir i, bw ) { return i.clone(); }
        static iv op_trunc( ir i, bw ) { return i.clone(); }
        static iv op_zext(  ir i, bw ) { return i.clone(); }
        static iv op_zfit(  ir i, bw ) { return i.clone(); }

        static iv op_eq ( ir a, ir b ) { /*dump(a); dump(b);*/ return a.get() == b.get(); }
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
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( __interval_cfg->choose_bound ) ) {
                auto i = __lart_choose( static_cast< int > ( bound_type( diff_low + 1 ) ) );
                a->meet_low( a.low() + i );
                b->meet_low( b.low() + diff_low - i );
            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( __interval_cfg->choose_bound ) ) {
                auto i = __lart_choose( static_cast< int > ( bound_type( diff_high + 1 ) ) );
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
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( __interval_cfg->choose_bound ) ) {
                auto i = __lart_choose( static_cast< int > ( bound_type( diff_low + 1 ) ) );
                a->meet_low( a.low() + i );
                b->meet_high( b.high() - diff_low + i );
            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( __interval_cfg->choose_bound ) ) {
                auto i = __lart_choose( static_cast< int > ( bound_type( diff_high + 1 ) ) );
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
            if ( diff_low > bound ( 0 ) && diff_low <= bound ( __interval_cfg->choose_bound ) ) {
                auto new_high = div_up( r.low(), b.low() );
                auto new_low = div_up( r.low(), b.high() );
                a->meet_low( new_low );
                auto i = __lart_choose( static_cast< int > ( bound_type( new_high - new_low + 1 ) ) );
                a->meet_low( new_high - i );
                b->meet_low( div_up( r.low(), ( new_high - i ) ) );
                if ( a.is_bottom() || b.is_bottom() )
                    __lart_cancel();

            }
            // upper bound correction
            if ( diff_high > bound( 0 ) && diff_high <= bound( __interval_cfg->choose_bound ) ) {
                auto new_low = std::max( r.high() / b.high(), a.low() );
                auto i = __lart_choose( static_cast< int > ( bound_type( a.high() - new_low + 1 ) ) );
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
            if ( interval_size > bound( 0 ) && interval_size <= bound( __interval_cfg->choose_bound ) ) {
                auto i = __lart_choose( static_cast< int > ( bound_type( interval_size ) ) );
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

            int size = static_cast< int > ( intersection.size() );
            if ( __interval_cfg->choose_bound < static_cast< const bound >( size ) )
                return;

            auto delim = __lart_choose( strict ? int(size + 1) : int(size) ) + intersection.low;
            a.intersect( { delim, a.high() } );
            b.intersect( { b.low(), strict ? delim - 1 : delim } );
        }

        static void beq( ir r, ref a, ref b, bool negated = false )
        {
            if ( r.get() == tristate( !negated ) ) beq_impl( a, b );
            if ( r.get() == tristate(  negated ) ) bne_impl( a, b );
        }

        static void bgt( ir r, ref a, ref b, bool negated = false )
        {
            if ( r.get() == tristate( !negated ) ) bgt_( a, b );
            if ( r.get() == tristate( negated ) ) bgt_( b, a, false /* non-strict */ );
        }

        static void bop_eq( ir r, ir a, ir b ) { beq( r, a, b ); }
        static void bop_ne( ir r, ir a, ir b ) {
            //printf("---\n");
            //dump(r); dump(a); dump(b);
            beq( r, a, b, true /* negated */ );
            //dump(a); dump(b); printf("---\n"); fflush(stdout);
        }

        static void bop_sgt( ir r, ir a, ir b ) { bgt( r, a, b ); }
        static void bop_slt( ir r, ir a, ir b ) { bgt( r, b, a ); }
        static void bop_sge( ir r, ir a, ir b ) { bgt( r, b, a, true /* negated */ ); }
        static void bop_sle( ir r, ir a, ir b ) { bgt( r, a, b, true /* negated */ ); }

        static void bop_ugt( ir r, ir a, ir b ) { bop_sgt( r, a, b ); }
        static void bop_uge( ir r, ir a, ir b ) { bop_sge( r, a, b ); }
        static void bop_ult( ir r, ir a, ir b ) { bop_slt( r, a, b ); }
        static void bop_ule( ir r, ir a, ir b ) { bop_sle( r, a, b ); }

        static void bop_zext( ir /*r*/, ir /*a*/ ) { /* noop */ }

        static void bop_zfit( ir r, ref a )
        {
            a.get() = r.get(); // TODO
        }

        static void dump( ir i )
        {
            printf( "%s\n", trace(i).c_str() );
        }

        static std::string trace( ir i )
        {
            std::stringstream ss;
            ss << '[' << i->low << ", " << i->high << ']';
            return ss.str();
        }

        template< typename stream >
        friend stream& operator<<( stream &os, ir i )
        {
            return os << trace(i);
        }
        /* ORIGINAL MEMOIZE
        static bool memoize( ir iref, void* twin, unsigned int line )
        {
            printf("Memoize (%s, %p, %u, %zu)\n", trace(iref).c_str(), twin, line, __interval_states->size);
            if ( __interval_states->max_size == 0 ) { return false; }
            if ( __interval_states->size == 0)
            {
                add_state( iref, twin, line );
                printf("Exit (%s, %p, %u, %zu)\n\n", trace(iref).c_str(), twin, line, __interval_states->size);
                return false; // memoized
            }
            int i = 0;
            while ( i < __interval_states->size && line > __interval_states->states[i].line ) { i++; }
            while ( i < __interval_states->size && twin > __interval_states->states[i].twin ) { i++; }
            print_states();
            printf("I = %i\n", i);
            auto state = __interval_states->states[i];
            printf("found state (%s, %p, %u)\n", trace(state.interval).c_str(), state.twin, state.line);
            if ( state.twin == twin && state.line == line )
            {
                printf("Similar state (%s, %p, %u)\n", trace(state.interval).c_str(), state.twin, state.line);
                if ( state.interval.includes( iref.get() ) )
                {
                    printf("State (%s, %p, %u) includes ", trace(state.interval).c_str(), state.twin, state.line);
                    printf("state (%s, %p, %u)\n", trace(iref).c_str(), twin, line);
                    return true; // memoized
                }
            }
            else
            {
                if ( __interval_states->size < __interval_states->max_size )
                {
                    add_state( iref, twin, line );
                }
            }
            printf("Exit (%s, %p, %u, %zu)\n\n", trace(iref).c_str(), twin, line, __interval_states->size);
            return false; // memoized
        }
        */



        /*
        void memoize_var( unsigned int line ){
            printf("I am at the end\n");
        }

        template< typename Arg, typename... Args >
        static void memoize_var( unsigned int line, Arg a, Args... args )
        {
            printf("Twin = %p\n", a);
        }
        */
        /*
        template < typename T, typename R >
        void memoize_var( unsigned int line, T& twins, R& refs )
        {
            printf("Line = %i\n", line);
            for( auto twin : twins ) {
                printf("Twin = %p\n", twin);
            }
            for( auto iref : refs ) {
                printf("Twin = %s\n", iref.get());
            }
            //size_t len = sizeof...(args);
            //printf("Line = %i; twins = %i\n", line, sizeof...(args));
        }
        */

        /*ORIGINAL MEMOIZE
        static void add_state( ir iref, void* twin, unsigned int line )
        {
            printf("Insert state (%s, %p, %u)\n", trace(iref).c_str(), twin, line);
            __interval_states->states[__interval_states->size].twin = twin;
            __interval_states->states[__interval_states->size].line = line;
            __interval_states->states[__interval_states->size].interval = iref.get();
            __interval_states->size++;
            qsort( __interval_states->states, __interval_states->size, sizeof(interval_state_t), cmpfunc );
        }

        static int cmpfunc ( const void * a, const void * b )
        {
            interval_state_t* first = (interval_state_t*)a;
            interval_state_t* second = (interval_state_t*)b;
            if ( (long)(first->twin) - (long)(second->twin) == 0 )
            {
                return first->line - second->line;
            }
            return (long)(first->twin) - (long)(second->twin);
        }

        static void print_states()
        {
            for (int i = 0; i < __interval_states->size; i++){
                auto state = __interval_states->states[i];
                printf("\tState (%s, %p, %u)\n", trace(state.interval).c_str(), state.twin, state.line);
            }
        }

    */
    };

    /*
    [[gnu::constructor]] void interval_setup()
    {
        __interval_cfg = (interval_config_t*)mmap(NULL, sizeof(interval_config_t), PROT_READ |
                                                PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        if ( auto opt = std::getenv( "INTERVAL_CHOOSE_BOUND" ); opt ) {
            fprintf( stderr, "[interval config] choose bound = %s\n", opt );
            __interval_cfg->choose_bound = std::atoi( opt );
        }
        //Najprv nainicializujem informacie o vsetkych stavoch
        __interval_states = (interval_states_t*)mmap(NULL, sizeof(interval_states_t),
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        if ( auto opt = std::getenv( "INTERVAL_MAX_STATES" ); opt ) {
            fprintf( stderr, "[interval states] max states = %s\n", opt );
            __interval_states->max_size = std::atoi( opt );
        }

        __interval_states->states = (interval_state_t*)mmap(NULL, sizeof(interval_state_t) * __interval_states->max_size,
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        //new ( __interval_state ) interval_state_t;
    }
    */
}
