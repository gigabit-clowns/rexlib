// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <rex/functional/creation.hpp>

#include "fixtures/cpu_execution_context_fixture.hpp"

#include <rex/core/meta/type_list.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array.hpp>
#include <rex/core/hardware/memory_resource_affinity.hpp>
#include <rex/core/numerical/numerical_type_traits.hpp>
#include <rex/core/numerical/scalar_value.hpp>

#include <complex>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace rexlib;

namespace
{

// TEMPLATE_LIST_TEST_CASE_METHOD requires the fixture to be a class template.
template <typename T>
class cpu_execution_context_fixture_tmpl : public cpu_execution_context_fixture
{
};

using all_types = type_list_cat_t<
	type_list<bool>,
	type_list<char>,
	type_list<std::int8_t, std::int16_t, std::int32_t, std::int64_t>,
	type_list<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>,
	type_list<float16_t, float32_t, float64_t>,
	type_list<
		std::complex<float16_t>, std::complex<float32_t>, std::complex<float64_t>
	>
>;

// The types arange enumerates: those on the real line, which is what it
// takes to ask a progression where it stops.
using real_arithmetic_types = type_list_cat_t<
	type_list<std::int8_t, std::int16_t, std::int32_t, std::int64_t>,
	type_list<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>,
	type_list<float16_t, float32_t, float64_t>
>;

// The types linspace samples with: those having a fractional part for the
// spacing to land in.
using inexact_types = type_list_cat_t<
	type_list<float16_t, float32_t, float64_t>,
	type_list<
		std::complex<float16_t>, std::complex<float32_t>, std::complex<float64_t>
	>
>;

} // namespace


TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"empty produces a host-accessible array carrying the requested descriptor",
	"[array_creation][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 });

	const auto result = empty(
		descriptor,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() == descriptor );

	// The storage must exist and be reachable from the host, even though its
	// contents are left uninitialized.
	const auto *storage = result.get_storage();
	REQUIRE( storage != nullptr );
	CHECK( storage->get_host_ptr() != nullptr );
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"zeros sets every element of a host array to zero",
	"[array_creation][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 2, 3 }, numerical_type_of<TestType>::value);

	const auto result = zeros(
		descriptor,
		memory_resource_affinity::host,
		this->context
	);

	CHECK( result.get_descriptor() == descriptor );

	for (const auto value : this->template read_host<TestType>(result, 6))
	{
		CHECK( value == static_cast<TestType>(0) );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"ones sets every element of a host array to one",
	"[array_creation][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 2, 3 }, numerical_type_of<TestType>::value);

	const auto result = ones(
		descriptor,
		memory_resource_affinity::host,
		this->context
	);

	CHECK( result.get_descriptor() == descriptor );

	for (const auto value : this->template read_host<TestType>(result, 6))
	{
		CHECK( value == static_cast<TestType>(1) );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"full sets every element of a host array to the requested value",
	"[array_creation][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 4 }, numerical_type_of<TestType>::value);

	const auto result = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(static_cast<TestType>(3)),
		this->context
	);

	for (const auto value : this->template read_host<TestType>(result, 4))
	{
		CHECK( value == static_cast<TestType>(3) );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"fill overwrites the contents of an existing host array",
	"[array_creation][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 5 }, numerical_type_of<TestType>::value);

	// Start from zeros so a successful fill is unambiguous.
	auto result = zeros(
		descriptor,
		memory_resource_affinity::host,
		this->context
	);

	fill(result, scalar_value(static_cast<TestType>(2)), this->context);

	for (const auto value : this->template read_host<TestType>(result, 5))
	{
		CHECK( value == static_cast<TestType>(2) );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange counts from zero up to the stop it is given",
	"[array_creation][cpu]"
)
{
	const auto result = arange(
		5,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 5 }, numerical_type::int32) );
	CHECK( read_host<std::int32_t>(result, 5) ==
	       std::vector<std::int32_t>{ 0, 1, 2, 3, 4 } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange counts from a start that is not zero",
	"[array_creation][cpu]"
)
{
	const auto result = arange(
		2,
		7,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);

	CHECK( read_host<std::int32_t>(result, 5) ==
	       std::vector<std::int32_t>{ 2, 3, 4, 5, 6 } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange stops before the stop it is given",
	"[array_creation][cpu]"
)
{
	// The range is half open, so a stop the step lands on exactly is left
	// out, and one it overshoots is not reached.
	const auto exact = arange(
		0,
		9,
		3,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);
	CHECK( read_host<std::int32_t>(exact, 3) ==
	       std::vector<std::int32_t>{ 0, 3, 6 } );

	const auto overshooting = arange(
		0,
		10,
		3,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);
	CHECK( read_host<std::int32_t>(overshooting, 4) ==
	       std::vector<std::int32_t>{ 0, 3, 6, 9 } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange counts down on a negative step",
	"[array_creation][cpu]"
)
{
	const auto result = arange(
		10,
		0,
		-3,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 4 }, numerical_type::int32) );
	CHECK( read_host<std::int32_t>(result, 4) ==
	       std::vector<std::int32_t>{ 10, 7, 4, 1 } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange takes a fractional step",
	"[array_creation][cpu]"
)
{
	const auto result = arange(
		0.0,
		1.0,
		0.25,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( read_host<float32_t>(result, 4) ==
	       std::vector<float32_t>{ 0.0F, 0.25F, 0.5F, 0.75F } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange produces an empty array when the step points away from the stop",
	"[array_creation][cpu]"
)
{
	const auto result = arange(
		0,
		5,
		-1,
		numerical_type::int32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 0 }, numerical_type::int32) );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange rejects a step that never advances",
	"[array_creation][cpu]"
)
{
	CHECK_THROWS_AS(
		arange(
			0,
			5,
			0,
			numerical_type::int32,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);

	CHECK_THROWS_AS(
		arange(
			0.0,
			5.0,
			0.0,
			numerical_type::float32,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange rejects an element type it cannot enumerate",
	"[array_creation][cpu]"
)
{
	// A progression has to be asked where it stops, which needs an ordering
	// the complex plane does not have, and a boolean is not a quantity to
	// count with.
	CHECK_THROWS_AS(
		arange(
			5,
			numerical_type::complex_float32,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);

	CHECK_THROWS_AS(
		arange(
			5,
			numerical_type::boolean,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"arange writes into an output array it is handed",
	"[array_creation][cpu]"
)
{
	auto destination = zeros(
		make_descriptor({ 4 }, numerical_type::int32),
		memory_resource_affinity::host,
		context
	);
	const auto *storage = destination.get_storage();

	const auto result = arange(
		1,
		5,
		numerical_type::int32,
		memory_resource_affinity::host,
		context,
		&destination
	);

	// The storage is the one that was already there, rather than a fresh
	// allocation the caller would have to go looking for.
	CHECK( result.get_storage() == storage );
	CHECK( read_host<std::int32_t>(result, 4) ==
	       std::vector<std::int32_t>{ 1, 2, 3, 4 } );
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"arange enumerates every type on the real line",
	"[array_creation][cpu]",
	real_arithmetic_types
)
{
	const auto result = arange(
		4,
		numerical_type_of<TestType>::value,
		memory_resource_affinity::host,
		this->context
	);

	const auto values = this->template read_host<TestType>(result, 4);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		INFO( "element " << i );
		CHECK( values[i] == static_cast<TestType>(i) );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace samples a segment including both of its ends",
	"[array_creation][cpu]"
)
{
	const auto result = linspace(
		0.0,
		1.0,
		5,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 5 }, numerical_type::float32) );
	CHECK( read_host<float32_t>(result, 5) ==
	       std::vector<float32_t>{ 0.0F, 0.25F, 0.5F, 0.75F, 1.0F } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace lands exactly on the stop even when the spacing does not",
	"[array_creation][cpu]"
)
{
	// A third of a segment is not representable, so an accumulated or
	// multiplied step would leave the far end a rounding short of it.
	const auto result = linspace(
		0.0,
		1.0,
		4,
		numerical_type::float64,
		memory_resource_affinity::host,
		context
	);

	const auto values = read_host<float64_t>(result, 4);
	CHECK( values.front() == 0.0 );
	CHECK( values.back() == 1.0 );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace leaves the stop out when asked",
	"[array_creation][cpu]"
)
{
	// The half open samples of one segment continue into those of the next
	// without repeating a value, which is what excluding the stop is for.
	const auto result = linspace(
		0.0,
		1.0,
		4,
		false,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( read_host<float32_t>(result, 4) ==
	       std::vector<float32_t>{ 0.0F, 0.25F, 0.5F, 0.75F } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace samples a descending segment",
	"[array_creation][cpu]"
)
{
	const auto result = linspace(
		1.0,
		0.0,
		5,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( read_host<float32_t>(result, 5) ==
	       std::vector<float32_t>{ 1.0F, 0.75F, 0.5F, 0.25F, 0.0F } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace gives a single sample the start rather than the stop",
	"[array_creation][cpu]"
)
{
	// One sample has no interval to span, so there is nothing for it to be
	// the far end of.
	const auto result = linspace(
		2.0,
		9.0,
		1,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 1 }, numerical_type::float32) );
	CHECK( read_host<float32_t>(result, 1) ==
	       std::vector<float32_t>{ 2.0F } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace produces an empty array when asked for no samples",
	"[array_creation][cpu]"
)
{
	const auto result = linspace(
		0.0,
		1.0,
		0,
		numerical_type::float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() ==
	       make_descriptor({ 0 }, numerical_type::float32) );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace samples a segment of the complex plane",
	"[array_creation][cpu]"
)
{
	using complex_type = std::complex<float32_t>;

	const auto result = linspace(
		complex_type(0.0F, 0.0F),
		complex_type(2.0F, 4.0F),
		3,
		numerical_type::complex_float32,
		memory_resource_affinity::host,
		context
	);

	CHECK( read_host<complex_type>(result, 3) ==
	       std::vector<complex_type>{
	           complex_type(0.0F, 0.0F),
	           complex_type(1.0F, 2.0F),
	           complex_type(2.0F, 4.0F)
	       } );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"linspace rejects an element type without a fractional part",
	"[array_creation][cpu]"
)
{
	// Evenly spaced samples are generally not whole numbers, so an integer
	// result would be a truncation dressed up as a sequence. arange is the
	// one that enumerates integers.
	CHECK_THROWS_AS(
		linspace(
			0.0,
			1.0,
			5,
			numerical_type::int32,
			memory_resource_affinity::host,
			context
		),
		std::invalid_argument
	);
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"linspace samples with every inexact type",
	"[array_creation][cpu]",
	inexact_types
)
{
	const auto result = linspace(
		0.0,
		4.0,
		5,
		numerical_type_of<TestType>::value,
		memory_resource_affinity::host,
		this->context
	);

	const auto values = this->template read_host<TestType>(result, 5);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		INFO( "element " << i );
		CHECK( values[i] == static_cast<TestType>(i) );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"copy duplicates the source contents into independent storage",
	"[array_creation][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 2, 3 }, numerical_type_of<TestType>::value);

	const auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(static_cast<TestType>(4)),
		this->context
	);

	const auto result = copy(source, this->context);

	CHECK( result.get_descriptor() == descriptor );

	// The copy owns a distinct buffer from the source.
	CHECK( result.get_storage() != source.get_storage() );

	for (const auto value : this->template read_host<TestType>(result, 6))
	{
		CHECK( value == static_cast<TestType>(4) );
	}
}
