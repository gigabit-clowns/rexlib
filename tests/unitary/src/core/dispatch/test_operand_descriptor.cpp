// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include "../hardware/mock/mock_buffer.hpp"

#include <memory>
#include <vector>

using namespace rexlib;

TEST_CASE(
	"Default constructed operand_descriptor should have an empty array "
	"descriptor and a null memory resource",
	"[operand_descriptor]"
)
{
	operand_descriptor operand;
	CHECK( operand.get_array_descriptor() == array_descriptor() );
	CHECK( operand.get_memory_resource() == nullptr );
	CHECK( operand.get_layout() == strided_layout() );
	CHECK( operand.get_data_type() == numerical_type::unknown );
}

TEST_CASE(
	"Constructing an operand_descriptor with an array descriptor and a "
	"memory resource should store its attributes",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const array_descriptor descriptor(layout, data_type);
	const auto* resource = &get_host_memory_resource();

	operand_descriptor operand(descriptor, resource);
	CHECK( operand.get_array_descriptor() == descriptor );
	CHECK( operand.get_memory_resource() == resource );
}

TEST_CASE(
	"Constructing an operand_descriptor with layout and data type should "
	"store its attributes",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const auto* resource = &get_host_memory_resource();

	operand_descriptor operand(layout, data_type, resource);
	CHECK( operand.get_layout() == layout );
	CHECK( operand.get_data_type() == data_type );
	CHECK( operand.get_memory_resource() == resource );
}

TEST_CASE(
	"Operand descriptors should be equal if the descriptor and memory "
	"resource are equal",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const array_descriptor descriptor(layout, data_type);
	const auto* resource = &get_host_memory_resource();

	const operand_descriptor operand1(descriptor, resource);
	const operand_descriptor operand2(descriptor, resource);

	CHECK( operand1 == operand2 );
	CHECK( (operand1 != operand2) == false );
}

TEST_CASE(
	"Operand descriptors should be unequal if descriptor or memory "
	"resource mismatch",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents1 = {20, 30, 2, 16};
	const std::vector<std::size_t> extents2 = {20, 30, 3, 16};
	const auto layout1 =
		strided_layout::make_contiguous_layout(make_span(extents1));
	const auto layout2 =
		strided_layout::make_contiguous_layout(make_span(extents2));
	const auto data_type = numerical_type::float32;
	const auto* resource = &get_host_memory_resource();

	const operand_descriptor operand1(layout1, data_type, resource);
	const operand_descriptor operand2(layout2, data_type, resource);
	const operand_descriptor operand3(layout1, data_type, nullptr);

	CHECK( operand1 != operand2 );
	CHECK( operand1 != operand3 );
	CHECK( operand2 != operand3 );
}

TEST_CASE(
	"hash value of two equal operand descriptors should be equal",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const auto* resource = &get_host_memory_resource();

	const operand_descriptor operand1(layout, data_type, resource);
	const operand_descriptor operand2(layout, data_type, resource);

	CHECK( operand1.hash() == operand2.hash() );
}

TEST_CASE(
	"hash value of operand descriptors should be different for different "
	"descriptors or memory resources",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const auto* resource = &get_host_memory_resource();

	const operand_descriptor operand1(layout, data_type, resource);
	const operand_descriptor operand2(layout, data_type, nullptr);
	const operand_descriptor operand3(layout, numerical_type::int32, resource);

	CHECK( operand1.hash() != operand2.hash() );
	CHECK( operand1.hash() != operand3.hash() );
}

TEST_CASE(
	"setters in operand_descriptor should update its attributes",
	"[operand_descriptor]"
)
{
	operand_descriptor operand;
	const std::vector<std::size_t> extents = {20, 30, 2, 16};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const array_descriptor descriptor(layout, data_type);
	const auto* resource = &get_host_memory_resource();

	operand.set_array_descriptor(descriptor);
	operand.set_memory_resource(resource);

	CHECK( operand.get_array_descriptor() == descriptor );
	CHECK( operand.get_memory_resource() == resource );
}

TEST_CASE(
	"from_array in operand_descriptor should correctly construct from an "
	"array and const_array",
	"[operand_descriptor]"
)
{
	const std::vector<std::size_t> extents = {20, 30};
	const auto layout =
		strided_layout::make_contiguous_layout(make_span(extents));
	const auto data_type = numerical_type::float32;
	const array_descriptor descriptor(layout, data_type);
	auto& resource = get_host_memory_resource();

	const auto storage = std::make_shared<mock_buffer>();
	ALLOW_CALL(std::as_const(*storage), get_memory_resource())
		.LR_RETURN(resource);

	array arr(storage, descriptor);
	const auto operand1 = operand_descriptor::from_array(arr);
	CHECK( operand1.get_array_descriptor() == descriptor );
	CHECK( operand1.get_memory_resource() == &resource );

	const const_array view = arr.share_const();
	const auto operand2 = operand_descriptor::from_array(view);
	CHECK( operand2.get_array_descriptor() == descriptor );
	CHECK( operand2.get_memory_resource() == &resource );
}
