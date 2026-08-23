// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/numerical/numerical_type_domain_tags.hpp>

#include <complex>
#include <cstdint>
#include <sstream>

using namespace rex;

TEST_CASE(
    "a default constructed numerical_type_domain should be empty",
    "[numerical_type_domain]"
)
{
    const numerical_type_domain domain;

    CHECK( domain.empty() );
    CHECK( domain.get_mask() == 0 );
    CHECK( !domain.contains(numerical_type::float32) );
}

TEST_CASE(
    "numerical_type_domain should contain exactly the listed types",
    "[numerical_type_domain]"
)
{
    const numerical_type_domain domain = {
        numerical_type::float32,
        numerical_type::complex_float64
    };

    CHECK( !domain.empty() );
    CHECK( domain.contains(numerical_type::float32) );
    CHECK( domain.contains(numerical_type::complex_float64) );
    CHECK( !domain.contains(numerical_type::float64) );
    CHECK( !domain.contains(numerical_type::boolean) );
}

TEST_CASE(
    "numerical_type_domain should ignore unrepresentable types",
    "[numerical_type_domain]"
)
{
    // numerical_type::unknown is negative, so it has no bit to set. It must
    // be dropped rather than shifting out of range.
    const numerical_type_domain domain = {
        numerical_type::unknown,
        numerical_type::int32
    };

    CHECK( domain.contains(numerical_type::int32) );
    CHECK( !domain.contains(numerical_type::unknown) );
    CHECK( !domain.contains(numerical_type::count) );
}

TEST_CASE(
    "is_domain_representable should accept only the concrete types",
    "[numerical_type_domain]"
)
{
    CHECK( !is_domain_representable(numerical_type::unknown) );
    CHECK( !is_domain_representable(numerical_type::count) );
    CHECK( is_domain_representable(numerical_type::boolean) );
    CHECK( is_domain_representable(numerical_type::complex_float64) );
}

TEST_CASE(
    "numerical_type_domain::all should contain every concrete type",
    "[numerical_type_domain]"
)
{
    const auto domain = numerical_type_domain::all();

    const auto count = static_cast<int>(numerical_type::count);
    for (int i = 0; i < count; ++i)
    {
        CHECK( domain.contains(static_cast<numerical_type>(i)) );
    }

    CHECK( !domain.contains(numerical_type::unknown) );
    CHECK( numerical_type_domain::none().empty() );
}

TEST_CASE(
    "numerical_type_domain set operations should behave as sets",
    "[numerical_type_domain]"
)
{
    const numerical_type_domain lhs = {
        numerical_type::int32,
        numerical_type::float32
    };
    const numerical_type_domain rhs = {
        numerical_type::float32,
        numerical_type::float64
    };

    const auto intersection = lhs & rhs;
    CHECK( intersection == numerical_type_domain{numerical_type::float32} );

    const auto union_ = lhs | rhs;
    CHECK( union_ == numerical_type_domain{
        numerical_type::int32,
        numerical_type::float32,
        numerical_type::float64
    } );

    CHECK( (~lhs).contains(numerical_type::float64) );
    CHECK( !(~lhs).contains(numerical_type::float32) );
    CHECK( (lhs | ~lhs) == numerical_type_domain::all() );
    CHECK( (lhs & ~lhs).empty() );
    CHECK( lhs != rhs );
}

TEST_CASE(
    "make_numerical_type_domain should group the types of a category",
    "[numerical_type_domain]"
)
{
    const auto domain = make_numerical_type_domain(
        numerical_type_category::floating_point
    );

    CHECK( domain.contains(numerical_type::float16) );
    CHECK( domain.contains(numerical_type::float32) );
    CHECK( domain.contains(numerical_type::float64) );
    CHECK( !domain.contains(numerical_type::complex_float32) );
    CHECK( !domain.contains(numerical_type::int32) );
}

TEST_CASE(
    "named category domains should match their category",
    "[numerical_type_domain]"
)
{
    CHECK( boolean_type_domain::get() ==
           numerical_type_domain{numerical_type::boolean} );

    CHECK( complex_type_domain::get() == numerical_type_domain{
        numerical_type::complex_float16,
        numerical_type::complex_float32,
        numerical_type::complex_float64
    } );

    CHECK( any_type_domain::get() == numerical_type_domain::all() );
}

TEST_CASE(
    "domain_union should combine its operands",
    "[numerical_type_domain]"
)
{
    CHECK( integral_type_domain::get() ==
           (signed_integer_type_domain::get() |
            unsigned_integer_type_domain::get()) );

    CHECK( inexact_type_domain::get() ==
           (floating_point_type_domain::get() |
            complex_type_domain::get()) );

    // Nesting a union inside another must flatten to the same set.
    CHECK( arithmetic_type_domain::get() ==
           (signed_integer_type_domain::get() |
            unsigned_integer_type_domain::get() |
            floating_point_type_domain::get() |
            complex_type_domain::get()) );
}

TEST_CASE(
    "named arithmetic domains should exclude boolean and character",
    "[numerical_type_domain]"
)
{
    // These are the domains that replace the hand written category checks,
    // so their exclusions are the load bearing part.
    const auto arithmetic = arithmetic_type_domain::get();
    CHECK( !arithmetic.contains(numerical_type::boolean) );
    CHECK( !arithmetic.contains(numerical_type::char8) );
    CHECK( arithmetic.contains(numerical_type::uint8) );
    CHECK( arithmetic.contains(numerical_type::complex_float32) );

    const auto real_arithmetic = real_arithmetic_type_domain::get();
    CHECK( real_arithmetic.contains(numerical_type::uint8) );
    CHECK( real_arithmetic.contains(numerical_type::float64) );
    CHECK( !real_arithmetic.contains(numerical_type::complex_float32) );

    const auto signed_arithmetic = signed_arithmetic_type_domain::get();
    CHECK( signed_arithmetic.contains(numerical_type::int32) );
    CHECK( signed_arithmetic.contains(numerical_type::complex_float32) );
    CHECK( !signed_arithmetic.contains(numerical_type::uint32) );
}

TEST_CASE(
    "domain_intersection should narrow its operands",
    "[numerical_type_domain]"
)
{
    // The way a backend narrows an operation's domain without restating it.
    using narrowed = domain_intersection<
        arithmetic_type_domain,
        real_arithmetic_type_domain
    >;

    CHECK( narrowed::get() == real_arithmetic_type_domain::get() );
    CHECK( !narrowed::get().contains(numerical_type::complex_float32) );
}

TEST_CASE(
    "a named domain should classify static types as it does values",
    "[numerical_type_domain]"
)
{
    // A domain answers the same question in two places: the rule engine
    // asks about a numerical_type value, and a backend asks about a static
    // type to decide whether to instantiate a kernel at all. Both read the
    // one constexpr mask, so they cannot drift.
    STATIC_REQUIRE(
        arithmetic_type_domain::contains_type<std::int32_t>::value
    );
    STATIC_REQUIRE(
        !arithmetic_type_domain::contains_type<bool>::value
    );
    STATIC_REQUIRE(
        !arithmetic_type_domain::contains_type<char>::value
    );
    STATIC_REQUIRE(
        arithmetic_type_domain::contains_type<std::complex<float32_t>>::value
    );

    STATIC_REQUIRE(
        !real_arithmetic_type_domain::contains_type<
            std::complex<float32_t>
        >::value
    );

    using no_booleans =
        domain_difference<any_type_domain, boolean_type_domain>;
    STATIC_REQUIRE( !no_booleans::contains_type<bool>::value );
    STATIC_REQUIRE( no_booleans::contains_type<char>::value );
    STATIC_REQUIRE( no_booleans::contains_type<float64_t>::value );

    CHECK( arithmetic_type_domain::get().contains(numerical_type::int32) ==
           arithmetic_type_domain::contains_type<std::int32_t>::value );
    CHECK( arithmetic_type_domain::get().contains(numerical_type::boolean) ==
           arithmetic_type_domain::contains_type<bool>::value );
}

TEST_CASE(
    "numerical_type_domain should be printable",
    "[numerical_type_domain]"
)
{
    std::ostringstream oss;
    oss << numerical_type_domain{
        numerical_type::float32,
        numerical_type::float64
    };
    CHECK( oss.str() == "float32, float64" );

    std::ostringstream empty_oss;
    empty_oss << numerical_type_domain::none();
    CHECK( empty_oss.str() == "<none>" );
}
