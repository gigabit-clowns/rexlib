// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/numerical/numerical_type_traits.hpp>

using namespace rex;

TEST_CASE( "numerical_type_of should contain the expected value", "[numerical_type_of]" )
{
    CHECK( numerical_type_of<char>::value == numerical_type::char8 );
    CHECK( numerical_type_of<std::int8_t>::value == numerical_type::int8 );
    CHECK( numerical_type_of<std::uint8_t>::value == numerical_type::uint8 );
    CHECK( numerical_type_of<std::int16_t>::value == numerical_type::int16 );
    CHECK( numerical_type_of<std::uint16_t>::value == numerical_type::uint16 );
    CHECK( numerical_type_of<std::int32_t>::value == numerical_type::int32 );
    CHECK( numerical_type_of<std::uint32_t>::value == numerical_type::uint32 );
    CHECK( numerical_type_of<std::int64_t>::value == numerical_type::int64 );
    CHECK( numerical_type_of<std::uint64_t>::value == numerical_type::uint64 );
    CHECK( numerical_type_of<float16_t>::value == numerical_type::float16 );
    CHECK( numerical_type_of<float32_t>::value == numerical_type::float32 );
    CHECK( numerical_type_of<float64_t>::value == numerical_type::float64 );
    CHECK( numerical_type_of<std::complex<float16_t>>::value == numerical_type::complex_float16 );
    CHECK( numerical_type_of<std::complex<float32_t>>::value == numerical_type::complex_float32 );
    CHECK( numerical_type_of<std::complex<float64_t>>::value == numerical_type::complex_float64 );
}

namespace
{

template <numerical_type Type, typename Expected>
struct maps_to
    : std::is_same<typename native_type_of<Type>::type, Expected>
{
};

template <numerical_type Type>
struct round_trips
    : std::integral_constant<
        bool,
        numerical_type_of<typename native_type_of<Type>::type>::value == Type
    >
{
};

} // namespace

TEST_CASE( "native_type_of should contain the expected type", "[native_type_of]" )
{
    STATIC_REQUIRE( maps_to<numerical_type::boolean, bool>::value );
    STATIC_REQUIRE( maps_to<numerical_type::char8, char>::value );
    STATIC_REQUIRE( maps_to<numerical_type::int8, std::int8_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::uint8, std::uint8_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::int16, std::int16_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::uint16, std::uint16_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::int32, std::int32_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::uint32, std::uint32_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::int64, std::int64_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::uint64, std::uint64_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::float16, float16_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::float32, float32_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::float64, float64_t>::value );
    STATIC_REQUIRE( maps_to<numerical_type::complex_float16, std::complex<float16_t>>::value );
    STATIC_REQUIRE( maps_to<numerical_type::complex_float32, std::complex<float32_t>>::value );
    STATIC_REQUIRE( maps_to<numerical_type::complex_float64, std::complex<float64_t>>::value );
}

TEST_CASE( "native_type_of should be the inverse of numerical_type_of", "[native_type_of]" )
{
    // The declarative type rules rely on this round trip to keep their
    // value level and type level halves in agreement, so it must hold for
    // every concrete numerical_type.
    STATIC_REQUIRE( round_trips<numerical_type::boolean>::value );
    STATIC_REQUIRE( round_trips<numerical_type::char8>::value );
    STATIC_REQUIRE( round_trips<numerical_type::int8>::value );
    STATIC_REQUIRE( round_trips<numerical_type::uint8>::value );
    STATIC_REQUIRE( round_trips<numerical_type::int16>::value );
    STATIC_REQUIRE( round_trips<numerical_type::uint16>::value );
    STATIC_REQUIRE( round_trips<numerical_type::int32>::value );
    STATIC_REQUIRE( round_trips<numerical_type::uint32>::value );
    STATIC_REQUIRE( round_trips<numerical_type::int64>::value );
    STATIC_REQUIRE( round_trips<numerical_type::uint64>::value );
    STATIC_REQUIRE( round_trips<numerical_type::float16>::value );
    STATIC_REQUIRE( round_trips<numerical_type::float32>::value );
    STATIC_REQUIRE( round_trips<numerical_type::float64>::value );
    STATIC_REQUIRE( round_trips<numerical_type::complex_float16>::value );
    STATIC_REQUIRE( round_trips<numerical_type::complex_float32>::value );
    STATIC_REQUIRE( round_trips<numerical_type::complex_float64>::value );
}
