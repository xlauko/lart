#pragma once

#include <cassert>
#include "tristate.hpp"

namespace sup
{
    template< typename underlying_t >
    struct bound
    {
        using value_t = underlying_t;

        static_assert( std::is_integral_v< value_t > );
        static_assert( std::is_signed_v< value_t > );

        value_t _value = 0;

        constexpr bound() = default;
        constexpr bound( value_t value ) : _value( value ) {}
        constexpr bound( const bound &o ) = default;
        constexpr bound( bound &&o ) = default;

        constexpr bound &operator=( const bound &o )
        {
            if ( this != &o )
                _value = o._value;
            return *this;
        }

        constexpr bound &operator=( bound &&o )
        {
            if ( this != &o )
                _value = std::move( o._value );
            return *this;
        }

        static constexpr value_t max() { return std::numeric_limits< value_t >::max(); }
        static constexpr value_t min() { return std::numeric_limits< value_t >::min(); }

        static constexpr bound plus_infinity()  { return max(); }
        static constexpr bound minus_infinity() { return min(); }

        constexpr bool is_infinite() const
        {
            return  _value == max() || _value == min();
        }

        constexpr bool is_finite() const { return !is_infinite(); }

        enum overflow { none = 0, plus = 1, minus = 2 };

        constexpr void increment() { ++_value; }

        constexpr void decrement() { --_value; }

        bound operator-() const
        {
            if ( _value == minus_infinity() )
                return plus_infinity();
            if ( _value == plus_infinity() )
                return minus_infinity();
            return -_value;
        }

        constexpr bound& operator+=( const bound &o )
        {
            auto overflows = [] ( auto a, auto b ) {
                if ( b > 0 && a > 0 )
                    return b >= max() - a ? overflow::plus : overflow::none;
                if ( b < 0 && a < 0 )
                    return b <= min() - a ? overflow::minus : overflow::none;
                return overflow::none;
            };

            if ( is_finite() && o.is_finite() ) {
                if ( auto ov = overflows( _value, o._value ); ov )
                    _value = ( ov == overflow::plus ) ? max() : min();
                else
                    _value += o._value;
            } else if ( is_finite() && o.is_infinite() ) {
                _value = o._value;
            } else if ( is_infinite() && o.is_finite() ) {
                // keep infinite value
            } else if ( o.is_infinite() && _value != o._value ) {
                __lart_cancel();
                //integer_fault( "addition of opposite infinities is undetermined" );
            }
            return *this;
        }

        constexpr bound& operator-=( const bound &o )
        {
            *this += -o;
            return *this;
        }

        constexpr bound& operator*=( const bound &o )
        {
            auto overflows = [] ( auto a, auto b ) {
                if ( a < 0 ) {
                    if ( b < 0 && -a > max() / -b )
                        return overflow::plus;
                    if ( b > 0 && a < min() / b )
                        return overflow::minus;
                }
                if ( a > 0 ) {
                    if ( b < 0 && b < min() / a )
                        return overflow::minus;
                    if ( b > 0 && a > max() / b )
                        return overflow::plus;
                }
                return overflow::none;
            };

            if ( o._value == 0 ) {
                _value = 0;
            } else if ( _value == 0 ) {
                // keep zero
            } else {
                if ( is_finite() && o.is_finite() )
                    if ( auto ov = overflows( _value, o._value ); ov )
                        _value = ( ov == overflow::plus ) ? max() : min();
                    else
                        _value *= o._value;
                else // is_infinite() || o.is_infinite()
                    _value = ( _value > 0 ) ^ ( o._value > 0 ) ? min() : max();
            }
            return *this;
        }

        bound& operator/=( const bound &o )
        {
            if ( is_finite() && o.is_finite() ) {
                _value /= o._value;
            } else if ( is_finite() && o.is_infinite() ) {
                _value = 0;
            } else if ( is_infinite() && o.is_finite() ) {
                if ( o._value < 0 )
                    *this = -(*this);
            } else {
                __lart_cancel();
                //integer_fault( "division of two infinities is undetermined" );
            }
            return *this;
        }

        bound& operator%=( const bound &o )
        {
            if ( is_finite() && o.is_finite() ) {
                _value %= o._value;
            }
            if ( o._value == 0 )
                __lart_cancel();
                //integer_fault( "reminder division with zero is undetermined" );
            return *this;
        }

        constexpr bound& operator&=( const bound &o ) {
            /* Note: ∞ is represented as MAX value of <_value> type.
                    -∞ is represented as MIN value of <_value> type. */
            _value &= o._value;
            return *this;
        }

        constexpr bound& operator|=( const bound &o ) {
            /* Note: ∞ is 0 and sequence of 1s
                    -∞ is 1 and sequence of 0s */
            _value |= o._value;
            return *this;
        }

        constexpr bound& operator^=( const bound &o )  {
            /* Note: ∞ is 0 and sequence of 1s
                    -∞ is 1 and sequence of 0s */
            _value ^= o.value;
            return *this;
        }

        constexpr bound& operator<<=( const bound &o ) {
            if ( o < bound( 0 ) )
                __lart_cancel();
                //integer_fault( "negative bitshift not defined" );
            else if ( is_infinite() && o.is_infinite() )
                __lart_cancel();
                //integer_fault( "infinite bitshifts not defined" );
            else if ( is_infinite() ) {}
                //keep infinite
            else if ( o == plus_infinity() )
                _value = 0;
            else
                _value <<= o._value;
            return *this;
        }

        constexpr bound& operator>>=( const bound &o ) {
            if ( o < bound( 0 ) )
                __lart_cancel();
                //integer_fault( "negative bitshift not defined" );
            else if ( is_infinite() && o.is_infinite() )
                __lart_cancel();
                //integer_fault( "infinite bitshifts not defined" );
            else if ( is_infinite() ) {}
                //keep infinite
            else if ( o == plus_infinity() )
                _value = max();
            else
                _value >>= o._value;
            return *this;
        }

        template< typename stream >
        friend stream& operator<<( stream &os, const bound& b )
        {
            if (b == minus_infinity())
                return os << "-∞";
            if (b == plus_infinity())
                return os << "-∞";
            return os << b._value;
        }

        friend constexpr bound operator+( bound l, bound r )   { return l += r; }
        friend constexpr bound operator+( value_t l, bound r ) { return bound( l ) += r; }
        friend constexpr bound operator+( bound l, value_t r ) { return l += bound( r ); }

        friend constexpr bound operator-( bound l, bound r )   { return l -= r; }
        friend constexpr bound operator-( value_t l, bound r ) { return bound( l ) -= r; }
        friend constexpr bound operator-( bound l, value_t r ) { return l -= bound( r ); }

        friend constexpr bound operator*( bound l, bound r )   { return l *= r; }
        friend constexpr bound operator*( value_t l, bound r ) { return bound( l ) *= r; }
        friend constexpr bound operator*( bound l, value_t r ) { return l *= bound( r ); }

        friend constexpr bound operator/( bound l, bound r )   { return l /= r; }
        friend constexpr bound operator/( value_t l, bound r ) { return bound( l ) /= r; }
        friend constexpr bound operator/( bound l, value_t r ) { return l /= bound( r ); }

        friend constexpr bound operator%( bound l, bound r )   { return l %= r; }
        friend constexpr bound operator%( value_t l, bound r ) { return bound( l ) %= r; }
        friend constexpr bound operator%( bound l, value_t r ) { return l %= bound( r ); }

        friend constexpr bool operator< ( bound l, bound r ) { return l._value < r._value; }
        friend constexpr bool operator> ( bound l, bound r ) { return l._value > r._value; }
        friend constexpr bool operator>=( bound l, bound r ) { return l._value >= r._value; }
        friend constexpr bool operator<=( bound l, bound r ) { return l._value <= r._value; }
        friend constexpr bool operator==( bound l, bound r ) { return l._value == r._value; }

        friend constexpr bound operator~( bound b ) { b._value = ~b._value; return b; }

        friend constexpr bound operator&( bound l, bound r ) { return l &= r; }
        friend constexpr bound operator&( value_t l, bound r ) { return bound( l ) &= r; }
        friend constexpr bound operator&( bound l, value_t r ) { return l &= bound( r ); }

        friend constexpr bound operator|( bound l, bound r ) { return l |= r; }
        friend constexpr bound operator|( value_t l, bound r ) { return bound ( l ) |= r; }
        friend constexpr bound operator|( bound l, value_t r ) { return l |= bound( r ); }

        friend constexpr bound operator^( bound l, bound r ) { return l ^= r; }
        friend constexpr bound operator^( value_t l, bound r ) { return bound( l ) ^= r; }
        friend constexpr bound operator^( bound l, value_t r ) { return l ^= bound( r ); }

        friend constexpr bound operator<<( bound l, bound r ) { return l <<= r; }
        friend constexpr bound operator<<( value_t l, bound r ) { return bound( l ) <<= r; }
        friend constexpr bound operator<<( bound l, value_t r ) { return l <<= bound( r ); }

        friend constexpr bound operator>>( bound l, bound r ) { return l >>= r; }
        friend constexpr bound operator>>( value_t l, bound r ) { return bound( l ) >>= r; }
        friend constexpr bound operator>>( bound l, value_t r ) { return l >>= bound( r ); }

        explicit operator underlying_t() const { return _value; }
    };

    template< typename bound >
    struct interval
    {
        using bound_t = bound;
        using value_t = typename bound_t::value_t;

        bound_t low;
        bound_t high;

        template< typename T >
        constexpr interval( const T &l, const T &h )
            : low( value_t(l) ), high( value_t(h) )
        {}

        template< typename T >
        constexpr interval( const std::pair< T, T > &p )
            : low( p.first ), high( p.second )
        {}

        template< typename T >
        constexpr interval( const interval< T > &i )
            : low( i.low ), high( i.high )
        {}

        template< typename T >
        constexpr interval( interval< T > &&i )
            : low( std::move( i.low ) ), high( std::move( i.high ) )
        {}

        constexpr interval( __lava::tristate t )
        {
            if ( __lava::maybe( t ) )
            {
                low = false;
                high = true;
            }
            else
            {
                low = high = static_cast< bool >( t );
            }
        }

        explicit constexpr interval( bool b )
            : low( b ), high( b )
        {}

        constexpr interval() = default;

        constexpr size_t size() const
        {
            return static_cast< size_t >( high._value ) - static_cast< size_t >( low._value ) + 1;
        }

        constexpr bool is_bottom() const { return high < low; }

        constexpr bool is_top() const { return low == bound_t::minus_infinity() && high == bound_t::plus_infinity(); }

        constexpr bool is_infinite() const { return low.is_infinite() || high.is_infinite(); }

        constexpr bool is_finite() const { return !is_infinite(); }

        constexpr bool constant() const { return low == high; }

        constexpr bool empty() const { return high < low; }

        constexpr bool includes( const bound_t &b ) const
        {
            return low <= b && b <= high;
        }

        constexpr bool includes( const interval &i ) const
        {
            return low <= i.low && i.high <= high;
        }

        constexpr bool intersects( const interval &i ) const
        {
            if ( i.low < low && i.high < low )
                return false;
            if ( low < i.low && high < i.low )
                return false;
            return true;
        }

        constexpr bool excludes( const bound_t &b ) const
        {
            return b < low || b > high;
        }

        constexpr bool excludes( const interval &i ) const
        {
            return i.high < low || high < i.low;
        }

        interval exclude( bound_t b ) const
        {
            if ( low == b )
                return { low + 1, high };
            if ( high == b )
                return { low, high - 1 };
            return *this;
        }

        explicit inline operator __lava::tristate() const noexcept
        {
            if ( low == 0 && high == 0 )
                return __lava::tristate( false );
            if ( includes( 0 ) )
                return __lava::tristate( __lava::maybe );
            return __lava::tristate( true );
        }

        constexpr void intersect( const interval &i )
        {
            *this = meet( *this, i );
        }

        friend constexpr interval meet( const interval &l, const interval &r ) // intersection
        {
            return { std::max( l.low, r.low ), std::min( l.high, r.high ) };
        }

        friend constexpr interval join( const interval &l, const interval &r ) // union
        {
            return { std::min( l.low, r.low ), std::max( l.high, r.high ) };
        }

        constexpr void meet_low( bound_t b )
        {
            low = std::max( low, b );
        }

        constexpr void meet_high( bound_t b )
        {
            high = std::min( high, b );
        }

        constexpr void join_low( bound_t b )
        {
            low = std::min( low, b );
        }

        constexpr void join_high( bound_t b )
        {
            low = std::max( low, b );
        }

        friend constexpr interval operator+( const interval &l, const interval &r )
        {
            return { l.low + r.low, l.high + r.high };
        }

        friend interval operator-( const interval &l, const interval &r )
        {
            return { l.low - r.high, l.high - r.low };
        }

        friend constexpr interval operator*( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  * r.low,
                l.low  * r.high,
                l.high * r.low,
                l.high * r.high
            } );
        }

        friend constexpr interval operator/( const interval &l, const interval &r ) //TODO deleted constexpr
        {
            if ( r.includes( 0 ) ) {
                __lart_cancel();
                //integer_fault( "Division by 0." );
                if ( r.low == 0 || r.high == 0 ) {
                    interval non_zero = r.exclude( static_cast< bound_t > (0) );
                    return l / non_zero;
                }



                interval int_neg = { r.low, static_cast< bound_t > (-1) };
                interval int_pos = { static_cast<bound_t> (1), r.high };
                return join( l / int_neg, l / int_pos );
            }

            return std::minmax( std::initializer_list< bound_t >{
                l.low  / r.low,
                l.low  / r.high,
                l.high / r.low,
                l.high / r.high
            } );
        }

        friend constexpr interval operator%( const interval &l, const interval &r )
        {
            if ( r.includes( 0 ) ) {
                __lart_cancel();
                //integer_fault( "division by zero" );
            }
            return std::minmax( std::initializer_list< bound_t >{
                l.low  % r.low,
                l.low  % r.high,
                l.high % r.low,
                l.high % r.high
            } );
        }

        friend constexpr interval operator>>( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  >> r.low,
                l.low  >> r.high,
                l.high >> r.low,
                l.high >> r.high
            } );
        }

        friend constexpr interval operator<<( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  << r.low,
                l.low  << r.high,
                l.high << r.low,
                l.high << r.high
            } );
        }

        friend constexpr interval operator|( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  | r.low,
                l.low  | r.high,
                l.high | r.low,
                l.high | r.high
            } );
        }

        friend constexpr interval operator&( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  & r.low,
                l.low  & r.high,
                l.high & r.low,
                l.high & r.high
            } );
        }

        friend constexpr interval operator^( const interval &l, const interval &r )
        {
            return std::minmax( std::initializer_list< bound_t >{
                l.low  ^ r.low,
                l.low  ^ r.high,
                l.high ^ r.low,
                l.high ^ r.high
            } );
        }

        friend constexpr interval operator~( const interval &t )
        {
            return std::minmax(std::initializer_list< bound_t >{
                ~t.low,
                ~t.high
            } );
        }

        friend constexpr __lava::tristate operator==( const interval &l , const interval &r )
        {
            if ( l.low == l.high && r.low == r.high && r.low == l.low )
                return __lava::tristate( true );
            return l.intersects( r ) ? __lava::maybe : __lava::tristate( false );
        }

        friend constexpr __lava::tristate operator==( const interval &l , __lava::tristate r )
        {
            return l == interval( r );
        }

        friend constexpr __lava::tristate operator==( __lava::tristate l , const interval &r )
        {
            return interval( l ) == r;
        }

        friend constexpr __lava::tristate operator!=( const interval &l , const interval &r )
        {
            return !( l == r );
        }

        friend constexpr __lava::tristate operator<( const interval &l, const interval &r )
        {
            if ( l.constant() && r.constant() )
                return __lava::tristate( l.low < r.low );
            if ( l.high < r.low )
                return __lava::tristate( true );
            if ( l.low > r.high ) //TODO - mal by som to zmenit na >=. Pre pripad ze by v tych hraniciach boli rovne
                return __lava::tristate( false );
            return __lava::maybe;
        }

        friend constexpr __lava::tristate operator>( const interval &l, const interval &r )
        {
            if ( l.constant() && r.constant() )
                return __lava::tristate( l.low > r.low );
            if ( l.low > r.high )
                return __lava::tristate( true );
            if ( l.high < r.low ) //TODO - mal by som to zmenit na <=. Pre pripad ze by v tych hraniciach boli rovne
                return __lava::tristate( false );
            return __lava::maybe;
        }

        friend constexpr __lava::tristate operator<=( const interval &l, const interval &r )
        {
            return !( l > r );
        }

        friend constexpr __lava::tristate operator>=( const interval &l, const interval &r )
        {
            return !( l < r );
        }
    };

    template< typename bound_t >
    interval( bound_t, bound_t ) -> interval< bound_t >;

    template< typename bound_t >
    interval( const std::pair< bound_t, bound_t > & ) -> interval< bound_t >;
}
