// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/rules/operand_type_transform.hpp>

#include <rex/core/numerical/numerical_type_dispatch.hpp>
#include <rex/core/numerical/numerical_type_traits.hpp>

#include <complex>
#include <type_traits>

using namespace rex;

namespace
{

// numerical_type_of is deliberately undefined for types that have no
// numerical_type, such as std::complex<int>. A transform's type level half
// is instantiated over every type by the dispatcher, including ones outside
// its domain, so detect the mapping rather than requiring it.
template <typename T, typename = void>
struct safe_numerical_type_of
    : std::integral_constant<numerical_type, numerical_type::unknown>
{
};

template <typename T>
struct safe_numerical_type_of<
    T,
    decltype(void(numerical_type_of<T>::value))
>
    : numerical_type_of<T>
{
};

/**
 * @brief Assert that a transform's two halves agree over its whole domain.
 *
 * This is the contract the declarative typing rules rest on: the runtime
 * policy reads `apply` and the backend dispatchers read `apply_type`, so if
 * the two ever disagree a rule would mean two different things on the two
 * sides of the dispatch boundary.
 */
template <typename Transform>
void check_transform_consistency()
{
    const auto &domain = Transform::domain_type::get();

    const auto count = static_cast<int>(numerical_type::count);
    for (int i = 0; i < count; ++i)
    {
        const auto type = static_cast<numerical_type>(i);
        if (!domain.contains(type))
        {
            continue;
        }

        const auto expected = Transform::apply(type);

        const auto got = dispatch_numerical_types(
            [] (auto tag)
            {
                using source_type = typename decltype(tag)::type;
                using result_type = typename Transform::template
                    apply_type<source_type>::type;
                return safe_numerical_type_of<result_type>::value;
            },
            type
        );

        INFO( "applied to " << type );
        CHECK( got == expected );
    }
}

} // anonymous namespace

TEST_CASE(
    "same_type_transform should agree on both of its halves",
    "[operand_type_transform]"
)
{
    check_transform_consistency<same_type_transform>();
}

TEST_CASE(
    "real_type_transform should agree on both of its halves",
    "[operand_type_transform]"
)
{
    check_transform_consistency<real_type_transform>();
}

TEST_CASE(
    "complex_type_transform should agree on both of its halves",
    "[operand_type_transform]"
)
{
    check_transform_consistency<complex_type_transform>();
}

TEST_CASE(
    "inexact_type_transform should agree on both of its halves",
    "[operand_type_transform]"
)
{
    check_transform_consistency<inexact_type_transform>();
}

TEST_CASE(
    "fixed_type_transform should agree on both of its halves",
    "[operand_type_transform]"
)
{
    check_transform_consistency<
        fixed_type_transform<numerical_type::boolean>
    >();
    check_transform_consistency<
        fixed_type_transform<numerical_type::float32>
    >();
    check_transform_consistency<
        fixed_type_transform<numerical_type::complex_float64>
    >();
}

TEST_CASE(
    "a transform domain should exclude the types it cannot map",
    "[operand_type_transform]"
)
{
    // make_complex is only defined on floating point and complex types, so
    // the transform must not claim to handle the rest. Were the domain
    // wrong, the consistency check above would already have caught it.
    const auto &domain = complex_type_transform::domain_type::get();
    CHECK( !domain.contains(numerical_type::int32) );
    CHECK( !domain.contains(numerical_type::boolean) );
    CHECK( domain.contains(numerical_type::float32) );
    CHECK( domain.contains(numerical_type::complex_float32) );

    // make_real, by contrast, is total.
    CHECK( real_type_transform::domain_type::get() ==
           numerical_type_domain::all() );

    // make_inexact has no numeric meaning for a character, so the domain
    // must exclude it while admitting every other type.
    const auto &inexact = inexact_type_transform::domain_type::get();
    CHECK( !inexact.contains(numerical_type::char8) );
    CHECK( inexact.contains(numerical_type::boolean) );
    CHECK( inexact.contains(numerical_type::int32) );
    CHECK( inexact.contains(numerical_type::float32) );
    CHECK( inexact.contains(numerical_type::complex_float32) );
}

TEST_CASE(
    "inexact_type_transform should widen exact types and keep inexact ones",
    "[operand_type_transform]"
)
{
    // Anything that can only hold whole values widens, so that the result of
    // an averaging operation is representable.
    CHECK( inexact_type_transform::apply(numerical_type::int32) ==
           numerical_type::float64 );
    CHECK( inexact_type_transform::apply(numerical_type::uint64) ==
           numerical_type::float64 );
    CHECK( inexact_type_transform::apply(numerical_type::boolean) ==
           numerical_type::float64 );

    // Types that already have a fractional part are left alone, so a
    // float32 operand is not silently widened to float64.
    CHECK( inexact_type_transform::apply(numerical_type::float16) ==
           numerical_type::float16 );
    CHECK( inexact_type_transform::apply(numerical_type::float32) ==
           numerical_type::float32 );
    CHECK( inexact_type_transform::apply(numerical_type::complex_float32) ==
           numerical_type::complex_float32 );

    STATIC_REQUIRE( std::is_same<
        inexact_type_transform::apply_type<std::int32_t>::type,
        float64_t
    >::value );
    STATIC_REQUIRE( std::is_same<
        inexact_type_transform::apply_type<float32_t>::type,
        float32_t
    >::value );
    STATIC_REQUIRE( std::is_same<
        inexact_type_transform::apply_type<std::complex<float32_t>>::type,
        std::complex<float32_t>
    >::value );
}

TEST_CASE(
    "real_type_transform should map complex types onto their real part",
    "[operand_type_transform]"
)
{
    CHECK( real_type_transform::apply(numerical_type::complex_float32) ==
           numerical_type::float32 );
    CHECK( real_type_transform::apply(numerical_type::int16) ==
           numerical_type::int16 );

    STATIC_REQUIRE( std::is_same<
        real_type_transform::apply_type<std::complex<float32_t>>::type,
        float32_t
    >::value );
    STATIC_REQUIRE( std::is_same<
        real_type_transform::apply_type<std::int16_t>::type,
        std::int16_t
    >::value );
}
