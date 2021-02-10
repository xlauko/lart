#pragma once

#include <cstdlib>
#include <runtime/lart.h>

namespace __lava
{
    /* The tristate abstract domain has three values: true, false and maybe
     * (unknown).  It is used for branching, to decide whether to take the then
     * or else path, or whether to choose non-deterministically (in DIVINE this
     * means split and take both paths).
     *
     * Every other abstract domain needs to define a `to_tristate()` method,
     * which specifies what happens if a value of that abstract type is
     * encountered in an if condition. */

    namespace detail
    {
        struct maybe_t {};
    } // namespace detail

    struct tristate;

    typedef bool ( *maybe_tag_t )( tristate, detail::maybe_t );

    constexpr inline bool maybe( tristate t, detail::maybe_t u = detail::maybe_t() ) noexcept;

    struct tristate
    {
        enum value_t : uint8_t { false_value = 0, true_value = 1, maybe_value = 2 } value;

        constexpr tristate() noexcept : value( false_value ) {}

        constexpr tristate( value_t v ) noexcept : value( v ) {}
        constexpr tristate( uint8_t v ) noexcept : tristate( value_t( v ) ) {}

        constexpr explicit tristate( bool v ) noexcept
            : value( v ? true_value : false_value )
        {}

        constexpr tristate( maybe_tag_t ) noexcept : value( maybe_value ) {}

        constexpr tristate( const tristate & ) = default;
        constexpr tristate( tristate && ) = default;

        constexpr explicit operator bool() const noexcept
        {
            return value == true_value;
        }
    };

    constexpr inline bool maybe( tristate t, detail::maybe_t ) noexcept
    {
        return t.value == tristate::maybe_value;
    }

    constexpr inline tristate operator!( tristate t ) noexcept
    {
        return t.value == tristate::false_value ? tristate( true )
             : t.value == tristate::true_value  ? tristate( false )
             : tristate( maybe );
    }

    constexpr inline tristate operator&&( tristate a, tristate b ) noexcept
    {
        return ( static_cast< bool >( !a ) || static_cast< bool >( !b ) )
             ? tristate( false )
             : ( ( static_cast< bool >( a ) && static_cast< bool >( b ) )
               ? tristate( true )
               : tristate( maybe ) );
    }

    constexpr inline tristate operator&&( tristate a, bool b ) noexcept
    {
        return b ? a : tristate( false );
    }

    constexpr inline tristate operator&&( bool a, tristate b ) noexcept
    {
        return a ? b : tristate( false );
    }

    constexpr inline tristate operator&&( maybe_tag_t, tristate b ) noexcept
    {
        return !b ? tristate( false ) : tristate( maybe );
    }

    constexpr inline tristate operator&&( tristate a, maybe_tag_t ) noexcept
    {
        return !a ? tristate( false ) : tristate( maybe );
    }

    constexpr inline tristate operator||( tristate a, tristate b ) noexcept
    {
        return ( static_cast< bool >( !a ) && static_cast< bool >( !b ) )
               ? tristate( false )
               : ( ( static_cast< bool >( a ) || static_cast< bool >( b ) )
                 ? tristate( true )
                 : tristate( maybe ) );
    }

    constexpr inline tristate operator||( tristate a, bool b ) noexcept
    {
        return b ? tristate( true ) : a;
    }

    constexpr inline tristate operator||( bool a, tristate b ) noexcept
    {
        return a ? tristate( true ) : b;
    }

    constexpr inline tristate operator||( maybe_tag_t, tristate b ) noexcept
    {
        return b ? tristate( true ) : tristate( maybe );
    }

    constexpr inline tristate operator||( tristate a, maybe_tag_t ) noexcept
    {
        return a ? tristate( true ) : tristate( maybe );
    }

    constexpr inline tristate operator==( tristate a, tristate b ) noexcept
    {
        return ( maybe( a ) || maybe( b ) )
               ? maybe
               : ( ( a && b ) || ( !a && !b ) );
    }

    constexpr inline tristate operator==( tristate a, bool b ) noexcept
    {
        return a == tristate( b );
    }

    constexpr inline tristate operator==( bool a, tristate b ) noexcept
    {
        return tristate( a ) == b;
    }

    constexpr inline tristate operator==( maybe_tag_t, tristate b ) noexcept
    {
        return maybe == b;
    }

    constexpr inline tristate operator==( tristate a, maybe_tag_t ) noexcept
    {
        return a == maybe;
    }

    constexpr inline tristate operator!=( tristate a, tristate b ) noexcept
    {
        return ( maybe( a ) || maybe( b ) )
               ? maybe
               : ( !( a && b ) || ( !a && !b ) );
    }

    constexpr inline tristate operator!=( tristate a, bool b ) noexcept
    {
        return a != tristate( b );
    }

    constexpr inline tristate operator!=( bool a, tristate b ) noexcept
    {
        return tristate( a ) != b;
    }

    constexpr inline tristate operator!=( maybe_tag_t, tristate b ) noexcept
    {
        return maybe != b;
    }

    constexpr inline tristate operator!=( tristate a, maybe_tag_t ) noexcept
    {
        return a != maybe;
    }

    template< typename stream >
    stream& operator<<( stream &os, const tristate &t )
    {
        auto to_string = [] ( auto v ) {
            switch ( v.value )
            {
            case tristate::true_value: return "true";
            case tristate::false_value: return "false";
            case tristate::maybe_value: return "maybe";
            }
        };

        return os << to_string( t );
    }

    constexpr inline tristate lift( bool v ) noexcept { return tristate( v ); }

    constexpr inline bool lower( tristate t ) noexcept
    {
        if ( t.value == tristate::maybe_value )
        {
            return __lart_choose(2);
        }
        return static_cast< bool >( t.value );
    }

} // namespace abstract
