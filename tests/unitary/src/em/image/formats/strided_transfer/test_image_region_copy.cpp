// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/strided_transfer/image_region_copy.hpp>

#include "../../fixtures/counting_image_file.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::test;

namespace
{

// Four images of 2 by 2 whose values count up from zero, so image n holds
// 4n, 4n + 1, 4n + 2 and 4n + 3.
const std::vector<std::size_t> stack_extents = {4, 2, 2};
const std::vector<std::size_t> image_extents = {2, 2};

const float untouched = -1.0F;

array make_counting_array(const std::vector<std::size_t> &extents)
{
	auto values =
		make_host_array<float>(extents, numerical_type::float32, 0.0F);
	auto *data = static_cast<float*>(values.get_storage()->get_host_ptr());
	for (std::size_t i = 0; i < count_elements(extents); ++i)
	{
		data[i] = static_cast<float>(i);
	}

	return values;
}

// Image `indices[i]` of a stack, landing in slot `i` of a batch.
image_transfer_plan images(const std::vector<std::size_t> &indices)
{
	image_transfer_plan plan(image_transfer_shape(image_extents, 3, 3));
	for (std::size_t slot = 0; slot < indices.size(); ++slot)
	{
		const std::size_t file_offset[3] = {indices[slot], 0, 0};
		const std::size_t array_offset[3] = {slot, 0, 0};
		plan.add(make_span(file_offset, 3), make_span(array_offset, 3));
	}

	return plan;
}

} // anonymous namespace

TEST_CASE(
	"copy_regions copies each region to where the plan places it",
	"[image_region_copy]"
)
{
	const auto source = make_counting_array(stack_extents);
	auto destination =
		make_host_array<float>({3, 2, 2}, numerical_type::float32, untouched);

	copy_regions(
		const_array_ref(source),
		array_ref(destination),
		images({3, 1})
	);

	// The third slot is named by no region and is left as it was.
	CHECK( get_values<float>(destination) == std::vector<float>({
		12, 13, 14, 15,
		4, 5, 6, 7,
		untouched, untouched, untouched, untouched
	}) );
}

TEST_CASE(
	"copy_regions converts to the data type of the destination",
	"[image_region_copy]"
)
{
	const auto source = make_counting_array(stack_extents);
	auto destination =
		make_host_array<double>({1, 2, 2}, numerical_type::float64, -1.0);

	copy_regions(const_array_ref(source), array_ref(destination), images({2}));

	CHECK( get_values<double>(destination) ==
		std::vector<double>({8, 9, 10, 11}) );
}

TEST_CASE(
	"copy_regions copies a region smaller than an image",
	"[image_region_copy]"
)
{
	const auto source = make_counting_array(stack_extents);
	auto destination =
		make_host_array<float>({1, 2}, numerical_type::float32, untouched);

	// The second row of image 2, into an array of one row.
	image_transfer_plan row(image_transfer_shape({1, 2}, 3, 2));
	const std::size_t file_offset[3] = {2, 1, 0};
	const std::size_t array_offset[2] = {0, 0};
	row.add(make_span(file_offset, 3), make_span(array_offset, 2));

	copy_regions(const_array_ref(source), array_ref(destination), row);

	CHECK( get_values<float>(destination) == std::vector<float>({10, 11}) );
}

TEST_CASE(
	"copy_regions reads a source that starts inside its buffer",
	"[image_region_copy]"
)
{
	// The last two images of the stack, as an array of their own over the
	// same buffer.
	auto stack = make_counting_array(stack_extents);
	const std::vector<std::size_t> extents = {2, 2, 2};
	const std::vector<std::ptrdiff_t> strides = {4, 2, 1};
	const array source(
		stack.share_storage(),
		array_descriptor(
			strided_layout::make_custom_layout(
				make_span(extents),
				make_span(strides),
				8
			),
			numerical_type::float32
		)
	);
	auto destination =
		make_host_array<float>({1, 2, 2}, numerical_type::float32, untouched);

	copy_regions(const_array_ref(source), array_ref(destination), images({1}));

	CHECK( get_values<float>(destination) ==
		std::vector<float>({12, 13, 14, 15}) );
}

TEST_CASE(
	"copy_regions of an empty plan copies nothing",
	"[image_region_copy]"
)
{
	const auto source = make_counting_array(stack_extents);
	auto destination =
		make_host_array<float>({1, 2, 2}, numerical_type::float32, untouched);

	copy_regions(const_array_ref(source), array_ref(destination), images({}));

	CHECK( get_values<float>(destination) ==
		std::vector<float>(4, untouched) );
}

TEST_CASE(
	"copy_regions refuses what it can not copy",
	"[image_region_copy]"
)
{
	const auto source = make_counting_array(stack_extents);
	auto destination =
		make_host_array<float>({1, 2, 2}, numerical_type::float32, untouched);

	SECTION( "a source that is not initialized" )
	{
		const array empty;

		REQUIRE_THROWS_AS(
			copy_regions(
				const_array_ref(empty),
				array_ref(destination),
				images({0})
			),
			std::invalid_argument
		);
	}

	SECTION( "a destination that is not initialized" )
	{
		array empty;

		REQUIRE_THROWS_AS(
			copy_regions(const_array_ref(source), array_ref(empty), images({})),
			std::invalid_argument
		);
	}

	SECTION( "a region the source does not contain" )
	{
		REQUIRE_THROWS_AS(
			copy_regions(
				const_array_ref(source),
				array_ref(destination),
				images({4})
			),
			std::out_of_range
		);
	}

	SECTION( "a region that does not fit in the destination" )
	{
		REQUIRE_THROWS_AS(
			copy_regions(
				const_array_ref(source),
				array_ref(destination),
				images({0, 1})
			),
			std::out_of_range
		);
	}
}
