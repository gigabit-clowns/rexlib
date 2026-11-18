// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/layouts/operand_core.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib::cpu;

TEST_CASE(
	"operand_core default constructs as an empty core",
	"[operand_core]"
)
{
	const operand_core core;

	CHECK( core.get_rank() == 0 );
	CHECK( core.get_extent(0) == 0 );
	CHECK( core.get_extent(1) == 0 );
	CHECK( core.get_stride(0) == 0 );
	CHECK( core.get_stride(1) == 0 );
}

TEST_CASE(
	"operand_core stores a rank two core as given",
	"[operand_core]"
)
{
	const operand_core core(2, { 3, 4 }, { 4, 1 });

	CHECK( core.get_rank() == 2 );
	CHECK( core.get_extent(0) == 3 );
	CHECK( core.get_extent(1) == 4 );
	CHECK( core.get_stride(0) == 4 );
	CHECK( core.get_stride(1) == 1 );
}

TEST_CASE(
	"operand_core stores a rank one core as given",
	"[operand_core]"
)
{
	const operand_core core(1, { 5, 0 }, { 2, 0 });

	CHECK( core.get_rank() == 1 );
	CHECK( core.get_extent(0) == 5 );
	CHECK( core.get_stride(0) == 2 );
}

TEST_CASE(
	"operand_core stores a rank three core as given",
	"[operand_core]"
)
{
	const operand_core core(3, { 8, 8, 5 }, { 40, 5, 1 });

	CHECK( core.get_rank() == 3 );
	CHECK( core.get_extent(2) == 5 );
	CHECK( core.get_stride(0) == 40 );
	CHECK( core.get_stride(2) == 1 );
}

TEST_CASE(
	"split_trailing_core peels the trailing axes off and keeps the rest as "
	"the batch",
	"[operand_core]"
)
{
	const std::vector<std::size_t> extents = { 2, 8, 8, 5 };
	const auto layout =
		rexlib::strided_layout::make_contiguous_layout(
			rexlib::make_span(extents)
		);
	std::vector<std::size_t> batch_extents;
	std::vector<std::ptrdiff_t> batch_strides;

	const auto core =
		split_trailing_core(layout, 3, batch_extents, batch_strides);

	CHECK( core.get_rank() == 3 );
	CHECK( core.get_extent(0) == 8 );
	CHECK( core.get_extent(1) == 8 );
	CHECK( core.get_extent(2) == 5 );
	CHECK( core.get_stride(0) == 40 );
	CHECK( core.get_stride(1) == 5 );
	CHECK( core.get_stride(2) == 1 );
	CHECK( batch_extents == std::vector<std::size_t>{ 2 } );
	CHECK( batch_strides == std::vector<std::ptrdiff_t>{ 320 } );
}

TEST_CASE(
	"split_trailing_core rejects a layout with fewer axes than the core",
	"[operand_core]"
)
{
	const std::vector<std::size_t> extents = { 8, 5 };
	const auto layout =
		rexlib::strided_layout::make_contiguous_layout(
			rexlib::make_span(extents)
		);
	std::vector<std::size_t> batch_extents;
	std::vector<std::ptrdiff_t> batch_strides;

	CHECK_THROWS_AS(
		split_trailing_core(layout, 3, batch_extents, batch_strides),
		std::invalid_argument
	);
}
