// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "../../core/hardware/mock/mock_memory_allocator.hpp"
#include "fixtures/counting_image_file.hpp"
#include "fixtures/scoped_path.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::test;

namespace
{

const std::size_t unlimited = std::numeric_limits<std::size_t>::max();

// Stacks of eight float32 images of 2 by 2, so one image takes 16 bytes.
const std::vector<std::size_t> stack_extents = {8, 2, 2};
const std::vector<std::size_t> image_extents = {2, 2};
const std::size_t image_size = 4;
const std::size_t image_bytes = image_size * sizeof(float);

const image_descriptor stack_descriptor(
	make_span(stack_extents),
	2,
	numerical_type::float32
);

using index_list = std::vector<std::size_t>;

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

std::vector<std::size_t> get_file_indices(const image_transfer_plan &plan)
{
	std::vector<std::size_t> indices;
	for (std::size_t region = 0; region < plan.get_region_count(); ++region)
	{
		indices.push_back(plan.get_file_offset(region).front());
	}

	return indices;
}

// An allocator that hands out host buffers of the size it is asked for.
std::unique_ptr<trompeloeil::expectation>
allow_allocating(mock_memory_allocator &allocator)
{
	return NAMED_ALLOW_CALL(
		allocator,
		allocate(trompeloeil::_, trompeloeil::_, trompeloeil::_)
	).RETURN( std::make_shared<host_buffer>(_1, _2) );
}

} // anonymous namespace

TEST_CASE(
	"a buffer_image_scratch holds an entry for each file that is named",
	"[buffer_image_scratch]"
)
{
	const auto first = std::make_shared<mock_image_reader>();
	const auto second = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const std::vector<image_location> locations = {
		image_location("stack_0.mrcs", 4),
		image_location("stack_1.mrcs", 2),
		image_location("stack_0.mrcs", 1)
	};

	ALLOW_CALL(*first, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*second, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(allocator, get_max_alignment()).RETURN(64);

	// Each file is opened once, and gets a buffer for the images held of it.
	REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(first);
	REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(second);
	REQUIRE_CALL(
		allocator,
		allocate(2 * image_bytes, sizeof(float), trompeloeil::_)
	).RETURN( std::make_shared<host_buffer>(_1, _2) );
	REQUIRE_CALL(
		allocator,
		allocate(image_bytes, sizeof(float), trompeloeil::_)
	).RETURN( std::make_shared<host_buffer>(_1, _2) );

	buffer_image_scratch scratch(
		make_span(locations),
		files,
		allocator,
		unlimited
	);

	SECTION( "an entry is found by the path of its file" )
	{
		CHECK( scratch.find("stack_0.mrcs") != nullptr );
		CHECK( scratch.find("stack_1.mrcs") != nullptr );
		CHECK( scratch.find("stack_0.mrcs") != scratch.find("stack_1.mrcs") );
	}

	SECTION( "a file that is not named has no entry" )
	{
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
	}

	SECTION( "a const scratch finds the same entries" )
	{
		const auto &const_scratch = scratch;

		CHECK( const_scratch.find("stack_0.mrcs") ==
			scratch.find("stack_0.mrcs") );
		CHECK( const_scratch.find("stack_2.mrcs") == nullptr );
	}
}

TEST_CASE(
	"a buffer_image_scratch holds every index of a file named as a whole",
	"[buffer_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 3),
		image_location("stack.mrcs")
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(allocator, get_max_alignment()).RETURN(64);
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);
	REQUIRE_CALL(
		allocator,
		allocate(8 * image_bytes, sizeof(float), trompeloeil::_)
	).RETURN( std::make_shared<host_buffer>(_1, _2) );

	const buffer_image_scratch scratch(
		make_span(locations),
		files,
		allocator,
		unlimited
	);

	CHECK( scratch.find("stack.mrcs") != nullptr );
}

TEST_CASE(
	"a buffer_image_scratch holds no more than its capacity",
	"[buffer_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const std::vector<image_location> locations = {
		image_location("stack_0.mrcs", 0),
		image_location("stack_0.mrcs", 1),
		image_location("stack_1.mrcs", 0),
		image_location("stack_1.mrcs", 1),
		image_location("stack_2.mrcs", 0)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(allocator, get_max_alignment()).RETURN(64);

	SECTION( "a file that fits whole leaves room for the next" )
	{
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_2.mrcs")).RETURN(reader);
		REQUIRE_CALL(
			allocator,
			allocate(2 * image_bytes, trompeloeil::_, trompeloeil::_)
		).TIMES(2).RETURN( std::make_shared<host_buffer>(_1, _2) );
		REQUIRE_CALL(
			allocator,
			allocate(image_bytes, trompeloeil::_, trompeloeil::_)
		).RETURN( std::make_shared<host_buffer>(_1, _2) );

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			5 * image_bytes
		);

		CHECK( scratch.find("stack_2.mrcs") != nullptr );
	}

	SECTION( "the first file that does not fit is cut, and none follows" )
	{
		// Room for three images and a half: two of the first stack and one
		// of the second. The third stack is not opened.
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_2.mrcs"));
		REQUIRE_CALL(
			allocator,
			allocate(2 * image_bytes, trompeloeil::_, trompeloeil::_)
		).RETURN( std::make_shared<host_buffer>(_1, _2) );
		REQUIRE_CALL(
			allocator,
			allocate(image_bytes, trompeloeil::_, trompeloeil::_)
		).RETURN( std::make_shared<host_buffer>(_1, _2) );

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			3 * image_bytes + image_bytes / 2
		);

		CHECK( scratch.find("stack_1.mrcs") != nullptr );
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
	}

	SECTION( "a file none of which fits has no entry" )
	{
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_1.mrcs"));
		FORBID_CALL(
			allocator,
			allocate(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		);

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			image_bytes - 1
		);

		CHECK( scratch.find("stack_0.mrcs") == nullptr );
	}

	SECTION( "no capacity opens no file" )
	{
		FORBID_CALL(files, acquire(trompeloeil::_));

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			0
		);

		CHECK( scratch.find("stack_0.mrcs") == nullptr );
	}
}

TEST_CASE(
	"a buffer_image_scratch refuses a stack index that a file does not have",
	"[buffer_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 8)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_MATCHES(
		buffer_image_scratch(make_span(locations), files, allocator, unlimited),
		std::out_of_range,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("stack.mrcs: buffer_image_scratch: ")
		)
	);
}

TEST_CASE(
	"a buffer_image_scratch reports what opening a file reported",
	"[buffer_image_scratch]"
)
{
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const std::vector<image_location> locations = {
		image_location("absent.mrcs", 0)
	};

	REQUIRE_CALL(files, acquire("absent.mrcs"))
		.SIDE_EFFECT( throw unsupported_operation_error("nothing claims it") )
		.RETURN(nullptr);

	REQUIRE_THROWS_AS(
		buffer_image_scratch(make_span(locations), files, allocator, unlimited),
		unsupported_operation_error
	);
}

TEST_CASE(
	"an entry of a buffer_image_scratch serves the images it holds",
	"[buffer_image_scratch]"
)
{
	const scoped_path path("buffer_scratch_serves.raw");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	mock_memory_allocator allocator;
	const auto allocating = allow_allocating(allocator);
	const std::vector<image_location> locations = {
		image_location(path.get(), 5),
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(allocator, get_max_alignment()).RETURN(64);
	REQUIRE_CALL(files, acquire(path.get())).RETURN(reader);

	SECTION( "all of them when they fit" )
	{
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			unlimited
		);
		const auto entry = scratch.find(path.get());
		auto destination = make_host_array<float>(
			{3, 2, 2},
			numerical_type::float32,
			-1.0F
		);

		// One run by default: one read of the file brings in all three.
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3, 5}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({5}));
		const auto missing =
			entry->read(array_ref(destination), images({5, 1, 3}));

		auto expected = count_from(5 * image_size, image_size);
		const auto second = count_from(1 * image_size, image_size);
		const auto third = count_from(3 * image_size, image_size);
		expected.insert(expected.end(), second.begin(), second.end());
		expected.insert(expected.end(), third.begin(), third.end());

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == expected );
	}

	SECTION( "the lowest indices when the capacity cuts the file" )
	{
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			2 * image_bytes
		);
		const auto entry = scratch.find(path.get());
		auto destination = make_host_array<float>(
			{3, 2, 2},
			numerical_type::float32,
			-1.0F
		);

		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({5, 1, 3}));
		const auto missing =
			entry->read(array_ref(destination), images({5, 1, 3}));

		CHECK( get_file_indices(missing) == index_list({5}) );
	}

	SECTION( "a run at a time when runs are bounded" )
	{
		// Runs of two images: indices 1 and 3 are one run, 5 another.
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			unlimited,
			2 * image_bytes
		);
		const auto entry = scratch.find(path.get());

		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({3}));
	}

	SECTION( "one index a run when a run is smaller than an image" )
	{
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			allocator,
			unlimited,
			1
		);
		const auto entry = scratch.find(path.get());

		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({3}));
	}
}
