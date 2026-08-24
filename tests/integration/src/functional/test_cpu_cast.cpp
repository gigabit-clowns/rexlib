// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/cast.hpp>

#include "fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/functional/creation.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>

#include <cstdint>
#include <vector>

using namespace rexlib;

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast aliases the input when it already has the requested type",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);

	auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(2.0f),
		context
	);

	const auto result = cast(source, numerical_type::float32, context);

	// The type already matches, so the result shares the source storage.
	CHECK( result.get_storage() == source.get_storage() );
	CHECK( result.get_descriptor() == descriptor );

	for (const auto value : read_host<float>(result, 6))
	{
		CHECK( value == 2.0f );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast converts the elements into an independent array when the type differs",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);

	auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(2.5f),
		context
	);

	const auto result = cast(source, numerical_type::float64, context);

	// A conversion is required, so a distinct buffer holding the target type is
	// produced.
	CHECK( result.get_storage() != source.get_storage() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );

	for (const auto value : read_host<double>(result, 6))
	{
		CHECK( value == 2.5 );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast truncates towards zero when converting to an integral type",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);

	auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(3.75f),
		context
	);

	const auto result = cast(source, numerical_type::int32, context);

	CHECK( result.get_descriptor().get_data_type() == numerical_type::int32 );

	for (const auto value : read_host<std::int32_t>(result, 6))
	{
		CHECK( value == 3 );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast_copy duplicates into independent storage even when the type matches",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);

	const auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(4.0f),
		context
	);

	const auto result = cast_copy(source, numerical_type::float32, context);

	// A forced copy always owns a distinct buffer from the source.
	CHECK( result.get_storage() != source.get_storage() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float32 );

	for (const auto value : read_host<float>(result, 6))
	{
		CHECK( value == 4.0f );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast_copy converts the elements to the requested type",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);

	const auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(7.0f),
		context
	);

	const auto result = cast_copy(source, numerical_type::float64, context);

	CHECK( result.get_storage() != source.get_storage() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );

	for (const auto value : read_host<double>(result, 6))
	{
		CHECK( value == 7.0 );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"cast_copy reuses the storage of the provided output array",
	"[array_cast][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 }, numerical_type::float32);
	const auto target_descriptor =
		make_descriptor({ 2, 3 }, numerical_type::float64);

	const auto source = full(
		descriptor,
		memory_resource_affinity::device,
		scalar_value(5.0f),
		context
	);

	// Pre-allocate a compatible output; its storage should be reused.
	auto out = zeros(
		target_descriptor,
		memory_resource_affinity::device,
		context
	);
	const auto *out_storage = out.get_storage();

	const auto result = cast_copy(
		source,
		numerical_type::float64,
		context,
		&out
	);

	CHECK( result.get_storage() == out_storage );
	CHECK( result.get_storage() != source.get_storage() );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float64 );

	for (const auto value : read_host<double>(result, 6))
	{
		CHECK( value == 5.0 );
	}
}
