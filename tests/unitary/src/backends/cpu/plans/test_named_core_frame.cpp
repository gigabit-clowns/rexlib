// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/plans/named_core_frame.hpp>

#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/span.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

std::vector<std::size_t> extents_of(const strided_layout &layout)
{
	std::vector<std::size_t> result;
	layout.get_extents(result);
	return result;
}

std::vector<std::ptrdiff_t> strides_of(const strided_layout &layout)
{
	std::vector<std::ptrdiff_t> result;
	layout.get_strides(result);
	return result;
}

strided_layout make_layout(
	std::vector<std::size_t> extents,
	std::vector<std::ptrdiff_t> strides,
	std::ptrdiff_t offset = 0
)
{
	return strided_layout::make_custom_layout(
		make_span(extents),
		make_span(strides),
		offset
	);
}

} // anonymous namespace

TEST_CASE(
	"named_core_frame should split the axes into core and batch axes, "
	"ascending",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> named = { 3, 1 };
	const named_core_frame frame({ 2, 3, 4, 5 }, make_span(named));

	CHECK(
		to_vector(frame.get_frame_extents()) ==
		std::vector<std::size_t>{ 2, 3, 4, 5 }
	);
	CHECK(
		to_vector(frame.get_core_axes()) == std::vector<std::size_t>{ 1, 3 }
	);
	CHECK(
		to_vector(frame.get_batch_axes()) == std::vector<std::size_t>{ 0, 2 }
	);
	CHECK( frame.get_core_extents() == std::vector<std::size_t>{ 3, 5 } );
	CHECK( frame.get_batch_extents() == std::vector<std::size_t>{ 2, 4 } );
}

TEST_CASE(
	"named_core_frame should reject an axis out of range or named twice",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> beyond = { 2 };
	const std::vector<std::size_t> twice = { 1, 1 };

	CHECK_THROWS_AS(
		named_core_frame({ 2, 3 }, make_span(beyond)),
		std::out_of_range
	);
	CHECK_THROWS_AS(
		named_core_frame({ 2, 3 }, make_span(twice)),
		std::invalid_argument
	);
}

TEST_CASE(
	"named_core_frame should select the values of the core and batch axes",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> named = { 0, 2 };
	const named_core_frame frame({ 2, 3, 4 }, make_span(named));
	const std::vector<std::ptrdiff_t> values = { 12, 4, 1 };

	CHECK(
		frame.select_core(make_span(values)) ==
		std::vector<std::ptrdiff_t>{ 12, 1 }
	);
	CHECK(
		frame.select_batch(make_span(values)) ==
		std::vector<std::ptrdiff_t>{ 4 }
	);
}

TEST_CASE(
	"named_core_frame should embed an operand carrying the core axes at the "
	"end of the frame",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> named = { 1, 2 };
	const named_core_frame frame({ 2, 4, 5 }, make_span(named));

	const auto embedded = frame.embed(make_layout({ 4, 5 }, { 5, 1 }, 3), true);

	CHECK( extents_of(embedded) == std::vector<std::size_t>{ 1, 4, 5 } );
	CHECK( strides_of(embedded) == std::vector<std::ptrdiff_t>{ 0, 5, 1 } );
	CHECK( embedded.get_offset() == 3 );
}

TEST_CASE(
	"named_core_frame should embed an operand without core axes at the end "
	"of the batch axes",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> trailing = { 1, 2 };
	const named_core_frame trailing_frame({ 2, 3, 4 }, make_span(trailing));
	const auto key = make_layout({ 2 }, { 1 }, 5);

	const auto behind = trailing_frame.embed(key, false);
	CHECK( extents_of(behind) == std::vector<std::size_t>{ 2, 1, 1 } );
	CHECK( strides_of(behind) == std::vector<std::ptrdiff_t>{ 1, 0, 0 } );
	CHECK( behind.get_offset() == 5 );

	const std::vector<std::size_t> around = { 0, 2 };
	const named_core_frame around_frame({ 3, 2, 4 }, make_span(around));

	const auto between = around_frame.embed(key, false);
	CHECK( extents_of(between) == std::vector<std::size_t>{ 1, 2, 1 } );
	CHECK( strides_of(between) == std::vector<std::ptrdiff_t>{ 0, 1, 0 } );
}

TEST_CASE(
	"named_core_frame should reject an operand without core axes having "
	"more axes than the batch",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> named = { 1, 2 };
	const named_core_frame frame({ 2, 3, 4 }, make_span(named));

	CHECK_THROWS_AS(
		frame.embed(make_layout({ 2, 3 }, { 3, 1 }), false),
		std::invalid_argument
	);
}

TEST_CASE(
	"named_core_frame should align an operand by broadcasting it to the "
	"frame",
	"[named_core_frame]"
)
{
	const std::vector<std::size_t> named = { 1, 2 };
	const named_core_frame frame({ 2, 3, 4 }, make_span(named));

	const auto scalar = frame.align(make_layout({}, {}), false);
	CHECK( extents_of(scalar) == std::vector<std::size_t>{ 2, 3, 4 } );
	CHECK( strides_of(scalar) == std::vector<std::ptrdiff_t>{ 0, 0, 0 } );

	const auto image = frame.align(make_layout({ 3, 4 }, { 4, 1 }), true);
	CHECK( extents_of(image) == std::vector<std::size_t>{ 2, 3, 4 } );
	CHECK( strides_of(image) == std::vector<std::ptrdiff_t>{ 0, 4, 1 } );

	CHECK_THROWS_AS(
		frame.align(make_layout({ 3 }, { 1 }), false),
		std::invalid_argument
	);
}
