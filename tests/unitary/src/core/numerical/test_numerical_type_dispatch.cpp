// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <rex/core/numerical/numerical_type_dispatch.hpp>

#include <array>
#include <typeindex>

using namespace rex;

TEST_CASE("dispatch_numerical_types called with a single argument and default mapping should invoke an appropriately typed function")
{
	numerical_type type;
	std::type_index expected_type(typeid(void));

	std::tie(type, expected_type) = GENERATE(
		table<numerical_type, std::type_index>({
			{ numerical_type::char8, typeid(char) },
			{ numerical_type::boolean, typeid(bool) },
			{ numerical_type::int8, typeid(std::int8_t)},
			{ numerical_type::uint8, typeid(std::uint8_t)},
			{ numerical_type::int16, typeid(std::int16_t)},
			{ numerical_type::uint16, typeid(std::uint16_t)},
			{ numerical_type::int32, typeid(std::int32_t)},
			{ numerical_type::uint32, typeid(std::uint32_t)},
			{ numerical_type::int64, typeid(std::int64_t)},
			{ numerical_type::uint64, typeid(std::uint64_t)},
			{ numerical_type::float16, typeid(float16_t)},
			{ numerical_type::float32, typeid(float32_t)},
			{ numerical_type::float64, typeid(float64_t)},
			{ numerical_type::complex_float16, typeid(std::complex<float16_t>)},
			{ numerical_type::complex_float32, typeid(std::complex<float32_t>)},
			{ numerical_type::complex_float64, typeid(std::complex<float64_t>)}
		})
	);

	const auto expected_ret = GENERATE(2, 42);

	const auto ret = dispatch_numerical_types(
		[&expected_type, expected_ret] (auto tag)
		{
			using type = typename decltype(tag)::type;
			REQUIRE( std::type_index(typeid(type)) == expected_type );
			return expected_ret;
		},
		type
	);

	CHECK( ret == expected_ret );
}

TEST_CASE("dispatch_numerical_types called with a two arguments and default mapping should invoke an appropriately typed function")
{
	std::array<numerical_type, 2> types;
	std::array<std::type_index, 2> expected_types = {
		typeid(void),
		typeid(void)
	};

	for (std::size_t i = 0; i < types.size(); ++i)
	{
		std::tie(types[i], expected_types[i]) = GENERATE(
			table<numerical_type, std::type_index>({
				{ numerical_type::char8, typeid(char) },
				{ numerical_type::boolean, typeid(bool) },
				{ numerical_type::int8, typeid(std::int8_t)},
				{ numerical_type::uint8, typeid(std::uint8_t)},
				{ numerical_type::int16, typeid(std::int16_t)},
				{ numerical_type::uint16, typeid(std::uint16_t)},
				{ numerical_type::int32, typeid(std::int32_t)},
				{ numerical_type::uint32, typeid(std::uint32_t)},
				{ numerical_type::int64, typeid(std::int64_t)},
				{ numerical_type::uint64, typeid(std::uint64_t)},
				{ numerical_type::float16, typeid(float16_t)},
				{ numerical_type::float32, typeid(float32_t)},
				{ numerical_type::float64, typeid(float64_t)},
				{ numerical_type::complex_float16, typeid(std::complex<float16_t>)},
				{ numerical_type::complex_float32, typeid(std::complex<float32_t>)},
				{ numerical_type::complex_float64, typeid(std::complex<float64_t>)}
			})
		);
	}

	const auto expected_ret = GENERATE(2, 42);

	const auto ret = dispatch_numerical_types(
		[&expected_types, expected_ret] (auto tag0, auto tag1)
		{
			using type0 = typename decltype(tag0)::type;
			using type1 = typename decltype(tag1)::type;
			REQUIRE( std::type_index(typeid(type0)) == expected_types[0] );
			REQUIRE( std::type_index(typeid(type1)) == expected_types[1] );
			return expected_ret;
		},
		types[0],
		types[1]
	);

	CHECK( ret == expected_ret );
}

TEST_CASE("dispatch_numerical_types called with an invalid type should throw")
{
	const auto type = GENERATE(
		numerical_type::unknown,
		numerical_type::count
	);

	REQUIRE_THROWS_MATCHES(
		dispatch_numerical_types([] (auto){}, type),
		std::invalid_argument,
		Catch::Matchers::Message(
			"Unknown numerical type"
		)
	);
}

