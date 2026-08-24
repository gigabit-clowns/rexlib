// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/roll_layout_plan.hpp>

#include <backends/cpu/loops/elementwise_loop.hpp>

#include <rexlib/core/span.hpp>

#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

TEST_CASE(
	"roll_axis_shift stores the axis and shift it is given",
	"[roll_axis_shift]"
)
{
	const roll_axis_shift shift(2, 5);

	CHECK( shift.get_axis() == 2 );
	CHECK( shift.get_shift() == 5 );
}

TEST_CASE(
	"build_roll_blocks makes one identity block when nothing is shifted",
	"[roll_layout_plan]"
)
{
	const std::vector<std::size_t> extents = { 4 };
	const std::vector<std::ptrdiff_t> output_strides = { 1 };
	const std::vector<std::ptrdiff_t> input_strides = { 1 };
	const std::vector<roll_axis_shift> shifts;

	const auto blocks = build_roll_blocks(
		make_span(extents),
		make_span(output_strides),
		10,
		make_span(input_strides),
		20,
		make_span(shifts)
	);

	REQUIRE( blocks.size() == 1 );
	CHECK( blocks[0].get_extents()[0] == 4 );
	CHECK( blocks[0].get_offset(0) == 10 );
	CHECK( blocks[0].get_offset(1) == 20 );
}

TEST_CASE(
	"build_roll_blocks splits a single shifted axis into two blocks",
	"[roll_layout_plan]"
)
{
	// A shift of 2 along an axis of extent 5: the output's first two
	// elements come from the input's last two (the wrapped part), and the
	// output's last three come from the input's first three.
	const std::vector<std::size_t> extents = { 5 };
	const std::vector<std::ptrdiff_t> output_strides = { 1 };
	const std::vector<std::ptrdiff_t> input_strides = { 1 };
	const std::vector<roll_axis_shift> shifts = { roll_axis_shift(0, 2) };

	const auto blocks = build_roll_blocks(
		make_span(extents),
		make_span(output_strides),
		0,
		make_span(input_strides),
		0,
		make_span(shifts)
	);

	REQUIRE( blocks.size() == 2 );

	// Output's tail (indices [2, 5)) reads the input's head ([0, 3)).
	CHECK( blocks[0].get_extents()[0] == 3 );
	CHECK( blocks[0].get_offset(0) == 2 );
	CHECK( blocks[0].get_offset(1) == 0 );

	// Output's head (indices [0, 2)) reads the input's tail ([3, 5)).
	CHECK( blocks[1].get_extents()[0] == 2 );
	CHECK( blocks[1].get_offset(0) == 0 );
	CHECK( blocks[1].get_offset(1) == 3 );
}

TEST_CASE(
	"build_roll_blocks combines strides and base offsets into each block's "
	"offset",
	"[roll_layout_plan]"
)
{
	const std::vector<std::size_t> extents = { 6 };
	const std::vector<std::ptrdiff_t> output_strides = { 3 };
	const std::vector<std::ptrdiff_t> input_strides = { 2 };
	const std::vector<roll_axis_shift> shifts = { roll_axis_shift(0, 4) };

	const auto blocks = build_roll_blocks(
		make_span(extents),
		make_span(output_strides),
		100,
		make_span(input_strides),
		50,
		make_span(shifts)
	);

	REQUIRE( blocks.size() == 2 );

	// Not wrapped: out_extra_offset = shift * output_stride.
	CHECK( blocks[0].get_extents()[0] == 2 );
	CHECK( blocks[0].get_offset(0) == 100 + 4 * 3 );
	CHECK( blocks[0].get_offset(1) == 50 );

	// Wrapped: in_extra_offset = (extent - shift) * input_stride.
	CHECK( blocks[1].get_extents()[0] == 4 );
	CHECK( blocks[1].get_offset(0) == 100 );
	CHECK( blocks[1].get_offset(1) == 50 + 2 * 2 );
}

TEST_CASE(
	"build_roll_blocks combines two shifted axes independently into "
	"2^2 blocks",
	"[roll_layout_plan]"
)
{
	const std::vector<std::size_t> extents = { 2, 3 };
	const std::vector<std::ptrdiff_t> output_strides = { 3, 1 };
	const std::vector<std::ptrdiff_t> input_strides = { 3, 1 };
	const std::vector<roll_axis_shift> shifts = {
		roll_axis_shift(0, 1), roll_axis_shift(1, 1)
	};

	const auto blocks = build_roll_blocks(
		make_span(extents),
		make_span(output_strides),
		0,
		make_span(input_strides),
		0,
		make_span(shifts)
	);

	REQUIRE( blocks.size() == 4 );

	// Every block partitions the 2x3 iteration space with no overlap or
	// gap, so their sizes add up to the whole. This holds regardless of
	// however joint_layout_builder orders each block's own axes.
	std::size_t total = 0;
	for (const auto &block : blocks)
	{
		std::size_t block_size = 1;
		for (const auto extent : block.get_extents())
		{
			block_size *= extent;
		}
		total += block_size;
	}
	CHECK( total == 6 );

	// Driving every block end to end is what actually exercises the
	// decomposition, and does not depend on how each block orders its own
	// axes: rolling both axes of the 2x3 sequence 0..5 by one apiece rolls
	// the rows down by one and the columns right by one.
	const std::vector<float> input = { 0, 1, 2, 3, 4, 5 };
	std::vector<float> output(6, -1.0f);

	struct assign_op
	{
		void operator()(float *out, const float *in) const noexcept
		{
			*out = *in;
		}
	};

	for (const auto &block : blocks)
	{
		run_elementwise_loop(assign_op{}, block, output.data(), input.data());
	}

	const std::vector<float> expected = { 5, 3, 4, 2, 0, 1 };
	CHECK( output == expected );
}
