// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <xmipp4/functional/transfer.hpp>

#include "fixtures/cpu_execution_context_fixture.hpp"

#include <xmipp4/functional/creation.hpp>
#include <xmipp4/core/meta/type_list.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/const_array.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/numerical/numerical_type_traits.hpp>
#include <xmipp4/core/numerical/scalar_value.hpp>

#include <complex>
#include <cstdint>

using namespace rex;

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

} // namespace


TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"transfer to device aliases the input on CPU because host and device "
	"resources coincide",
	"[array_transfer][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 });

	auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(2.0f),
		context
	);

	const auto result = transfer(
		source,
		memory_resource_affinity::device,
		context
	);

	CHECK( result.get_descriptor() == descriptor );

	// On CPU the device resource aliases the host resource, so no copy is made:
	// the result shares the source storage.
	CHECK( result.get_storage() == source.get_storage() );

	const auto values = read_host<float>(result, 6);
	for (const auto value : values)
	{
		CHECK( value == 2.0f );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"transfer to host aliases the input on CPU",
	"[array_transfer][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 });

	auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(5.0f),
		context
	);

	const auto result = transfer(
		source,
		memory_resource_affinity::host,
		context
	);

	CHECK( result.get_descriptor() == descriptor );

	// The source already lives on host-accessible memory, so it is aliased.
	CHECK( result.get_storage() == source.get_storage() );

	const auto values = read_host<float>(result, 6);
	for (const auto value : values)
	{
		CHECK( value == 5.0f );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"transfer_copy duplicates the source into independent storage on CPU",
	"[array_transfer][cpu]",
	all_types
)
{
	const auto descriptor =
		this->make_descriptor({ 2, 3 }, numerical_type_of<TestType>::value);

	const auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(static_cast<TestType>(3)),
		this->context
	);

	const auto result = transfer_copy(
		source,
		memory_resource_affinity::device,
		this->context
	);

	CHECK( result.get_descriptor() == descriptor );

	// A forced copy always owns a distinct buffer from the source, even though
	// the CPU host and device resources coincide.
	CHECK( result.get_storage() != source.get_storage() );

	for (const auto value : this->template read_host<TestType>(result, 6))
	{
		CHECK( value == static_cast<TestType>(3) );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"transfer_copy reuses the storage of the provided output array on CPU",
	"[array_transfer][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 });

	const auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(6.0f),
		context
	);

	// Pre-allocate a compatible output; its storage should be reused.
	auto out = zeros(
		descriptor,
		memory_resource_affinity::host,
		context
	);
	const auto *out_storage = out.get_storage();

	const auto result = transfer_copy(
		source,
		memory_resource_affinity::host,
		context,
		&out
	);

	CHECK( result.get_storage() == out_storage );
	CHECK( result.get_storage() != source.get_storage() );

	const auto values = read_host<float>(result, 6);
	for (const auto value : values)
	{
		CHECK( value == 6.0f );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"to_device and to_host alias the input on CPU",
	"[array_transfer][cpu]"
)
{
	const auto descriptor = make_descriptor({ 2, 3 });

	auto source = full(
		descriptor,
		memory_resource_affinity::host,
		scalar_value(1.5f),
		context
	);

	const auto on_device = to_device(source, context);
	CHECK( on_device.get_storage() == source.get_storage() );

	const auto on_host = to_host(source, context);
	CHECK( on_host.get_storage() == source.get_storage() );

	const auto values = read_host<float>(on_device, 6);
	for (const auto value : values)
	{
		CHECK( value == 1.5f );
	}
}

TEMPLATE_LIST_TEST_CASE_METHOD(
	cpu_execution_context_fixture_tmpl,
	"to_device_copy and to_host_copy duplicate into independent storage on CPU",
	"[array_transfer][cpu]",
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

	const auto on_device = to_device_copy(source, this->context);
	CHECK( on_device.get_storage() != source.get_storage() );

	const auto on_host = to_host_copy(source, this->context);
	CHECK( on_host.get_storage() != source.get_storage() );
	CHECK( on_host.get_storage() != on_device.get_storage() );

	for (const auto value : this->template read_host<TestType>(on_device, 6))
	{
		CHECK( value == static_cast<TestType>(4) );
	}
	for (const auto value : this->template read_host<TestType>(on_host, 6))
	{
		CHECK( value == static_cast<TestType>(4) );
	}
}
