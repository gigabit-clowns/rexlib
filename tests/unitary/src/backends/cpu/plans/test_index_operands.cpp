// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/index_operands.hpp>

#include <rexlib/core/layout/joint_layout.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/span.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

joint_layout_builder make_builder(
	const std::vector<std::size_t> &extents,
	const std::vector<std::ptrdiff_t> &strides
)
{
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	builder.add_operand(make_span(extents), make_span(strides), 0);
	return builder;
}

std::vector<std::ptrdiff_t> strides_of(
	const joint_layout &layout,
	std::size_t operand
)
{
	const auto strides = layout.get_strides(operand);
	return std::vector<std::ptrdiff_t>(strides.begin(), strides.end());
}

} // anonymous namespace

TEST_CASE(
	"add_index_operands adds nothing for a traversal handing out no index",
	"[index_operands]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	auto builder = make_builder(extents, { 3, 1 });

	add_index_operands(builder, make_span(extents), no_index_tag());

	CHECK( builder.build().get_number_of_operands() == 1 );
}

TEST_CASE(
	"add_index_operands strides the linear index in row-major order",
	"[index_operands]"
)
{
	const std::vector<std::size_t> extents = { 2, 3, 4 };
	auto builder = make_builder(extents, { 1, 2, 6 });

	add_index_operands(builder, make_span(extents), linear_index_tag());
	const auto layout = builder.build({});

	REQUIRE( layout.get_number_of_operands() == 2 );
	CHECK( strides_of(layout, 1) == std::vector<std::ptrdiff_t>{ 12, 4, 1 } );
	CHECK( layout.get_offset(1) == 0 );
}

TEST_CASE(
	"add_index_operands strides the coordinate of every axis along that axis "
	"alone",
	"[index_operands]"
)
{
	const std::vector<std::size_t> extents = { 2, 3, 4 };
	auto builder = make_builder(extents, { 12, 4, 1 });

	add_index_operands(
		builder,
		make_span(extents),
		multidimensional_index_tag()
	);
	const auto layout = builder.build({});

	REQUIRE( layout.get_number_of_operands() == 4 );
	CHECK( strides_of(layout, 1) == std::vector<std::ptrdiff_t>{ 1, 0, 0 } );
	CHECK( strides_of(layout, 2) == std::vector<std::ptrdiff_t>{ 0, 1, 0 } );
	CHECK( strides_of(layout, 3) == std::vector<std::ptrdiff_t>{ 0, 0, 1 } );
	CHECK( layout.get_offset(1) == 0 );
	CHECK( layout.get_offset(2) == 0 );
	CHECK( layout.get_offset(3) == 0 );
}

TEST_CASE(
	"index operands leave the order of the axes to the other operands",
	"[index_operands]"
)
{
	// A row-major operand is walked with its last axis innermost.
	const std::vector<std::size_t> extents = { 2, 3, 4 };
	auto builder = make_builder(extents, { 12, 4, 1 });

	add_index_operands(
		builder,
		make_span(extents),
		multidimensional_index_tag()
	);
	const auto layout = builder.build();

	REQUIRE( layout.get_rank() == 3 );
	CHECK( strides_of(layout, 0) == std::vector<std::ptrdiff_t>{ 1, 4, 12 } );
	CHECK( strides_of(layout, 1) == std::vector<std::ptrdiff_t>{ 0, 0, 1 } );
	CHECK( strides_of(layout, 3) == std::vector<std::ptrdiff_t>{ 1, 0, 0 } );
}

TEST_CASE(
	"a linear index lets the axes of a row-major operand coalesce",
	"[index_operands]"
)
{
	const std::vector<std::size_t> extents = { 2, 3, 4 };
	auto builder = make_builder(extents, { 12, 4, 1 });

	add_index_operands(builder, make_span(extents), linear_index_tag());
	const auto layout = builder.build();

	REQUIRE( layout.get_rank() == 1 );
	CHECK( strides_of(layout, 0) == std::vector<std::ptrdiff_t>{ 1 } );
	CHECK( strides_of(layout, 1) == std::vector<std::ptrdiff_t>{ 1 } );
}

TEST_CASE(
	"a linear index keeps the axes of a column-major operand apart, each "
	"with the strides it started with",
	"[index_operands]"
)
{
	// Coalescing the operand would leave no stride the linear index could
	// advance by.
	const std::vector<std::size_t> extents = { 2, 3, 4 };
	auto builder = make_builder(extents, { 1, 2, 6 });

	add_index_operands(builder, make_span(extents), linear_index_tag());
	const auto layout = builder.build();

	REQUIRE( layout.get_rank() == 3 );

	const auto operand_strides = strides_of(layout, 0);
	const auto index_strides = strides_of(layout, 1);
	std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> axes;
	for (std::size_t axis = 0; axis < layout.get_rank(); ++axis)
	{
		axes.emplace_back(operand_strides[axis], index_strides[axis]);
	}
	std::sort(axes.begin(), axes.end());

	const std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> expected = {
		{ 1, 12 },
		{ 2, 4 },
		{ 6, 1 }
	};
	CHECK( axes == expected );
}

TEST_CASE(
	"coordinates still let an axis of extent one be dropped",
	"[index_operands]"
)
{
	const std::vector<std::size_t> extents = { 3, 1, 2 };
	auto builder = make_builder(extents, { 2, 2, 1 });

	add_index_operands(
		builder,
		make_span(extents),
		multidimensional_index_tag()
	);
	const auto layout = builder.build();

	CHECK( layout.get_rank() == 2 );
}
