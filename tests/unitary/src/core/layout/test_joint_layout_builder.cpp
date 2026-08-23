// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <xmipp4/core/layout/joint_layout_builder.hpp>

#include <xmipp4/core/layout/strided_layout.hpp>
#include <xmipp4/core/exceptions/invalid_operation_error.hpp>
#include <xmipp4/core/layout/broadcast_error.hpp>
#include <xmipp4/core/layout/joint_layout.hpp>
#include <core/layout/joint_layout_implementation.hpp>

#include <algorithm>
#include <iostream>

using namespace rex;

TEST_CASE( "default constructing a joint_layout_builder should point to a null implementation", "[joint_layout_builder]" )
{
	joint_layout_builder builder;
	const auto *impl = builder.get_implementation();
	REQUIRE( impl == nullptr );
}

TEST_CASE( "setting the iteration extents in joint_layout_builder should initialize the implementation", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result = impl->get_extents();
	REQUIRE( std::equal(extents.cbegin(), extents.cend(), result.begin(), result.end()) );
}

TEST_CASE( "setting the iteration extents in joint_layout_builder with an implementation should throw", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));

	REQUIRE_THROWS_MATCHES(
		builder.set_extents(xmipp4::make_span(extents)),
		xmipp4::invalid_operation_error,
		Catch::Matchers::Message("Extents can only be set once and before adding any operand")
	);
}

TEST_CASE( "adding the first operand joint_layout_builder should initialize it", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents));

	builder.add_operand(layout);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result = impl->get_extents();
	CHECK( std::equal(extents.cbegin(), extents.cend(), result.begin(), result.end()) );
}

TEST_CASE( "Adding a valid operand joint_layout_builder should add it", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides;
	layout.get_strides(strides);
	const auto offset = layout.get_offset();

	builder.set_extents(xmipp4::make_span(extents));
	builder.add_operand(layout);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto operand_extents = impl->get_extents();
	CHECK( std::equal(extents.cbegin(), extents.cend(), operand_extents.begin(), operand_extents.end()) );
	REQUIRE( impl->get_number_of_operands() == 1 );
	const auto operand_strides = impl->get_strides(0);
	CHECK( std::equal(strides.cbegin(), strides.cend(), operand_strides.begin(), operand_strides.end()) );
	CHECK( impl->get_offset(0) == offset );
}

TEST_CASE( "Adding an operand with non-broadcastable extents in joint_layout_builder should throw", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents1 = {20, 6, 12, 12};
	std::vector<std::size_t> extents2 = {20, 4, 12, 12};
	auto layout = strided_layout::make_contiguous_layout(xmipp4::make_span(extents1));

	builder.set_extents(xmipp4::make_span(extents2));

	REQUIRE_THROWS_MATCHES(
		builder.add_operand(layout),
		std::invalid_argument,
		Catch::Matchers::Message(
			"Unable to broadcast extent of 6 into target extent of 4."
		)
	);
}

TEST_CASE( "Adding an operand with a size-1 axis in joint_layout_builder should broadcast it", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> iteration_extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(iteration_extents));

	const std::vector<std::size_t> operand_extents = {20, 1, 12, 12};
	const std::vector<std::ptrdiff_t> operand_strides = { 144, 144, 12, 1 };
	const auto operand_layout = strided_layout::make_custom_layout(
		xmipp4::make_span(operand_extents),
		xmipp4::make_span(operand_strides)
	);
	builder.add_operand(operand_layout);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result_extents = impl->get_extents();
	REQUIRE( std::equal(iteration_extents.cbegin(), iteration_extents.cend(), result_extents.begin(), result_extents.end()) );
	REQUIRE( impl->get_number_of_operands() == 1 );
	const std::vector<std::ptrdiff_t> expected_strides = { 144, 0, 12, 1 };
	const auto result_strides = impl->get_strides(0);
	CHECK( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides.begin(), result_strides.end()) );
}

TEST_CASE( "Adding an operand with fewer dimensions in joint_layout_builder should pad with size-1 axes", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> iteration_extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(iteration_extents));

	const std::vector<std::size_t> operand_extents = {12, 12};
	const std::vector<std::ptrdiff_t> operand_strides = { 12, 1 };
	const auto operand_layout = strided_layout::make_custom_layout(
		xmipp4::make_span(operand_extents),
		xmipp4::make_span(operand_strides)
	);
	builder.add_operand(operand_layout);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result_extents = impl->get_extents();
	REQUIRE( std::equal(iteration_extents.cbegin(), iteration_extents.cend(), result_extents.begin(), result_extents.end()) );
	REQUIRE( impl->get_number_of_operands() == 1 );
	const std::vector<std::ptrdiff_t> expected_strides = { 0, 0, 12, 1 };
	const auto result_strides = impl->get_strides(0);
	CHECK( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides.begin(), result_strides.end()) );
}

TEST_CASE( "Adding an operand with fewer dimensions and a size-1 axis in joint_layout_builder should broadcast both", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> iteration_extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(iteration_extents));

	const std::vector<std::size_t> operand_extents = {1, 12};
	const std::vector<std::ptrdiff_t> operand_strides = { 12, 1 };
	const auto operand_layout = strided_layout::make_custom_layout(
		xmipp4::make_span(operand_extents),
		xmipp4::make_span(operand_strides)
	);
	builder.add_operand(operand_layout);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result_extents = impl->get_extents();
	REQUIRE( std::equal(iteration_extents.cbegin(), iteration_extents.cend(), result_extents.begin(), result_extents.end()) );
	REQUIRE( impl->get_number_of_operands() == 1 );
	const std::vector<std::ptrdiff_t> expected_strides = { 0, 0, 0, 1 };
	const auto result_strides = impl->get_strides(0);
	CHECK( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides.begin(), result_strides.end()) );
}

TEST_CASE( "Adding a broadcastable operand via strides and extents in joint_layout_builder should work", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> iteration_extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(iteration_extents));

	const std::vector<std::size_t> operand_extents = {20, 1, 12, 12};
	const std::vector<std::ptrdiff_t> operand_strides = { 144, 0, 12, 1 };
	const std::ptrdiff_t offset = 7;
	builder.add_operand(
		xmipp4::make_span(operand_extents),
		xmipp4::make_span(operand_strides),
		offset
	);

	const auto *impl = builder.get_implementation();
	REQUIRE( impl );
	const auto result_extents = impl->get_extents();
	REQUIRE( std::equal(iteration_extents.cbegin(), iteration_extents.cend(), result_extents.begin(), result_extents.end()) );
	REQUIRE( impl->get_number_of_operands() == 1 );
	const std::vector<std::ptrdiff_t> expected_strides = { 144, 0, 12, 1 };
	const auto result_strides = impl->get_strides(0);
	CHECK( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides.begin(), result_strides.end()) );
	CHECK( impl->get_offset(0) == offset );
}

TEST_CASE("build on joint_layout_builder should move the implementation", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	const auto *impl = builder.get_implementation();

	auto layout = builder.build();
	REQUIRE( builder.get_implementation() == nullptr );
	REQUIRE( layout.get_implementation() == impl );
}

TEST_CASE("build with enable_reordering on joint_layout_builder should re-order axes such that the first operand appears in column major ordering", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides1 = { 864, 144, 12, 1 };
	const auto operand_layout1 = strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides1));
	builder.add_operand(operand_layout1);
	std::vector<std::ptrdiff_t> strides2 = { 1, 20, 120, 1440 };
	const auto operand_layout2 = strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides2));
	builder.add_operand(operand_layout2);

	auto layout = builder.build(joint_layout_build_flag_bits::enable_reordering);

	const std::vector<std::size_t> expected_extents = {12, 12, 6, 20};
	const std::vector<std::ptrdiff_t> expected_strides1 = { 1, 12, 144, 864 };
	const std::vector<std::ptrdiff_t> expected_strides2 = { 1440, 120, 20, 1 };
	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(expected_extents.cbegin(), expected_extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(expected_strides1.cbegin(), expected_strides1.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(expected_strides2.cbegin(), expected_strides2.cend(), result_strides2.begin(), result_strides2.end()) );
}

TEST_CASE("build with enable_coalescing on joint_layout_builder should coalesce contiguous axes", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides = { 1, 20, 120, 1440 };
	const auto operand_layout = 
			strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides));
	builder.add_operand(operand_layout);
	builder.add_operand(operand_layout);

	auto layout = builder.build(joint_layout_build_flag_bits::enable_coalescing);

	const std::vector<std::size_t> expected_extents = { 17280 };
	const std::vector<std::ptrdiff_t> expected_strides = { 1 };
	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(expected_extents.cbegin(), expected_extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides2.begin(), result_strides2.end()) );
}

TEST_CASE("build with enable_coalescing on joint_layout_builder should coalesce phantom axes", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 1, 1, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides = { 1, 2, 4, 20 };
	const auto operand_layout = 
			strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides));
	builder.add_operand(operand_layout);
	builder.add_operand(operand_layout);

	auto layout = builder.build(joint_layout_build_flag_bits::enable_coalescing);

	const std::vector<std::size_t> expected_extents = { 240 };
	const std::vector<std::ptrdiff_t> expected_strides = { 1 };
	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(expected_extents.cbegin(), expected_extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides2.begin(), result_strides2.end()) );
}

TEST_CASE("build with enable_coalescing on joint_layout_builder should not coalesce non-contiguous axes", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides1 = { 1, 40, 240, 2880 }; // Non-contiguous 2nd axis
	const auto operand_layout1 = 
		strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides1));
	std::vector<std::ptrdiff_t> strides2 = { 1, 20, 120, 2880 }; // Non-contiguous 4th axis
	const auto operand_layout2 = 
		strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides2));
	builder.add_operand(operand_layout1);
	builder.add_operand(operand_layout2);

	auto layout = builder.build(joint_layout_build_flag_bits::enable_coalescing);

	const std::vector<std::size_t> expected_extents = { 20, 72, 12 };
	const std::vector<std::ptrdiff_t> expected_strides1 = { 1, 40, 2880 };
	const std::vector<std::ptrdiff_t> expected_strides2 = { 1, 20, 2880 };
	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(expected_extents.cbegin(), expected_extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(expected_strides1.cbegin(), expected_strides1.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(expected_strides2.cbegin(), expected_strides2.cend(), result_strides2.begin(), result_strides2.end()) );
}

TEST_CASE("build on joint_layout_builder without flags should not modify the layout", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides = { 1, 20, 120, 1440 };
	const auto operand_layout = 
		strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides));
	builder.add_operand(operand_layout);
	builder.add_operand(operand_layout);

	auto layout = builder.build({});

	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(extents.cbegin(), extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(strides.cbegin(), strides.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(strides.cbegin(), strides.cend(), result_strides2.begin(), result_strides2.end()) );
}

TEST_CASE( "build with default flags on joint_layout should re-order and coalesce contiguous axes", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));
	std::vector<std::ptrdiff_t> strides = { 1728, 288, 12, 1 };
	const auto operand_layout = strided_layout::make_custom_layout(xmipp4::make_span(extents), xmipp4::make_span(strides));
	builder.add_operand(operand_layout);
	builder.add_operand(operand_layout);

	auto layout = builder.build();

	const std::vector<std::size_t> expected_extents = { 144, 120 };
	const std::vector<std::ptrdiff_t> expected_strides = { 1, 288 };
	const auto result_extents = layout.get_extents();
	REQUIRE( std::equal(expected_extents.cbegin(), expected_extents.cend(), result_extents.begin(), result_extents.end()) );
	const auto result_strides1 = layout.get_strides(0);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides1.begin(), result_strides1.end()) );
	const auto result_strides2 = layout.get_strides(1);
	REQUIRE( std::equal(expected_strides.cbegin(), expected_strides.cend(), result_strides2.begin(), result_strides2.end()) );
}


TEST_CASE( "building a reduce operation in joint_layout_builder should produce expected axis ordering", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));

	auto result_strides = GENERATE(
		std::vector<std::ptrdiff_t>{ 12, 0, 1, 0 },
		std::vector<std::ptrdiff_t>{ 0, 12, 0, 1 },
		std::vector<std::ptrdiff_t>{ 0, 12, 0, 1 },
		std::vector<std::ptrdiff_t>{ 12, 0, 0, 1 },
		std::vector<std::ptrdiff_t>{ 0, 0, 24, 1 }
	);
	const auto result_layout = 
		strided_layout::make_custom_layout(
			xmipp4::make_span(extents), 
			xmipp4::make_span(result_strides)
		);
	builder.add_operand(result_layout);

	const std::vector<std::ptrdiff_t> input_strides = { 3456, 288, 12, 1 };
	const auto input_layout = 
		strided_layout::make_custom_layout(
			xmipp4::make_span(extents), 
			xmipp4::make_span(input_strides)
		);
	builder.add_operand(input_layout);

	auto layout = builder.build();

	const auto compiled_extents = layout.get_extents();
	CHECK( std::equal(extents.crbegin(), extents.crend(), compiled_extents.begin(), compiled_extents.end()) );

	const auto compiled_result_strides = layout.get_strides(0);
	CHECK( std::equal(result_strides.crbegin(), result_strides.crend(), compiled_result_strides.begin(), compiled_result_strides.end()) );

	const auto compiled_input_strides = layout.get_strides(1);
	CHECK( std::equal(input_strides.crbegin(), input_strides.crend(), compiled_input_strides.begin(), compiled_input_strides.end()) );
}

TEST_CASE( "building a reduce operation in joint_layout_builder should simplify when possible", "[joint_layout_builder]" )
{
	joint_layout_builder builder;

	const std::vector<std::size_t> extents = {20, 6, 12, 12};
	builder.set_extents(xmipp4::make_span(extents));

	std::vector<std::ptrdiff_t> result_strides = { 0, 0, 12, 1 };
	const auto result_layout = 
		strided_layout::make_custom_layout(
			xmipp4::make_span(extents), 
			xmipp4::make_span(result_strides)
		);
	builder.add_operand(result_layout);

	const std::vector<std::ptrdiff_t> input_strides = { 864, 144, 12, 1 };
	const auto input_layout = 
		strided_layout::make_custom_layout(
			xmipp4::make_span(extents), 
			xmipp4::make_span(input_strides)
		);
	builder.add_operand(input_layout);

	auto layout = builder.build();

	const std::vector<std::size_t> expected_extents = {144, 120};
	const auto compiled_extents = layout.get_extents();
	CHECK( std::equal(expected_extents.cbegin(), expected_extents.cend(), compiled_extents.begin(), compiled_extents.end()) );

	const std::vector<std::ptrdiff_t> expected_result_strides = {1, 0};
	const auto compiled_result_strides = layout.get_strides(0);
	CHECK( std::equal(expected_result_strides.cbegin(), expected_result_strides.cend(), compiled_result_strides.begin(), compiled_result_strides.end()) );

	const std::vector<std::ptrdiff_t> expected_input_strides = {1, 144};
	const auto compiled_input_strides = layout.get_strides(1);
	CHECK( std::equal(expected_input_strides.cbegin(), expected_input_strides.cend(), compiled_input_strides.begin(), compiled_input_strides.end()) );
}
