// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/buffer_image_scratch.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "fixtures/counting_image_file.hpp"
#include "fixtures/scoped_path.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

#include <core/hardware/host_memory/host_buffer.hpp>
#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
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

// A host buffer of a number of bytes, aligned for float32.
std::shared_ptr<buffer> make_storage(std::size_t size)
{
	return std::make_shared<host_buffer>(size, sizeof(float));
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

// Every image of a stack, as one region.
image_transfer_plan whole_stack()
{
	image_transfer_plan plan(image_transfer_shape(stack_extents, 3, 3));
	const std::size_t offset[3] = {0, 0, 0};
	plan.add(make_span(offset, 3), make_span(offset, 3));

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

// Where in its buffer an array starts, in elements.
std::ptrdiff_t get_first_element(array_ref values)
{
	return values.get_descriptor().get_layout().get_offset();
}

} // anonymous namespace

TEST_CASE(
	"a buffer_image_scratch needs a buffer the host can reach",
	"[buffer_image_scratch]"
)
{
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	SECTION( "a null buffer is refused" )
	{
		REQUIRE_THROWS_AS(
			buffer_image_scratch(make_span(locations), files, nullptr),
			std::invalid_argument
		);
	}

	SECTION( "a buffer the host can not reach is refused" )
	{
		const auto storage = std::make_shared<mock_buffer>();
		ALLOW_CALL(*storage, get_host_ptr()).RETURN(nullptr);

		REQUIRE_THROWS_AS(
			buffer_image_scratch(make_span(locations), files, storage),
			unsupported_capability_error
		);
	}
}

TEST_CASE(
	"a buffer_image_scratch holds an entry for each file that is named",
	"[buffer_image_scratch]"
)
{
	const auto first = std::make_shared<mock_image_reader>();
	const auto second = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack_0.mrcs", 4),
		image_location("stack_1.mrcs", 2),
		image_location("stack_0.mrcs", 1)
	};

	ALLOW_CALL(*first, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*second, get_descriptor()).RETURN(std::ref(stack_descriptor));

	// Each file is opened once, however many locations name it.
	REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(first);
	REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(second);

	buffer_image_scratch scratch(
		make_span(locations),
		files,
		make_storage(8 * image_bytes)
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

	SECTION( "an entry holds the indices that the locations name" )
	{
		REQUIRE_CALL(*first, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 4}) );
		REQUIRE_CALL(*second, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({2}) );

		scratch.find("stack_0.mrcs")->store(*first, whole_stack());
		scratch.find("stack_1.mrcs")->store(*second, whole_stack());
	}

	SECTION( "the entries are stored one after another in the buffer" )
	{
		// Two images of four elements, then one image.
		REQUIRE_CALL(*first, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_first_element(_1) == 0 );
		REQUIRE_CALL(*second, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_first_element(_1) == 2 * image_size );

		scratch.find("stack_0.mrcs")->store(*first, whole_stack());
		scratch.find("stack_1.mrcs")->store(*second, whole_stack());
	}
}

TEST_CASE(
	"a buffer_image_scratch holds every index of a file named as a whole",
	"[buffer_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 3),
		image_location("stack.mrcs")
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			get_file_indices(_2) == index_list({0, 1, 2, 3, 4, 5, 6, 7})
		);

	buffer_image_scratch scratch(
		make_span(locations),
		files,
		make_storage(8 * image_bytes)
	);

	scratch.find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"a buffer_image_scratch aligns each entry for the data type of its file",
	"[buffer_image_scratch]"
)
{
	// Three rows of three bytes, then a stack of float32 images.
	const std::vector<std::size_t> bytes_extents = {3, 3};
	const image_descriptor bytes_descriptor(
		make_span(bytes_extents),
		2,
		numerical_type::uint8
	);
	const auto bytes = std::make_shared<mock_image_reader>();
	const auto stack = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("bytes.mrc"),
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*bytes, get_descriptor()).RETURN(std::ref(bytes_descriptor));
	ALLOW_CALL(*stack, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("bytes.mrc")).RETURN(bytes);
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(stack);

	buffer_image_scratch scratch(
		make_span(locations),
		files,
		make_storage(64)
	);

	// Nine bytes are taken, so the float32 values start at the twelfth:
	// their third element.
	REQUIRE_CALL(*stack, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_first_element(_1) == 3 );

	scratch.find("stack.mrcs")->store(*stack, whole_stack());
}

TEST_CASE(
	"a buffer_image_scratch refuses a buffer that is not aligned for a file",
	"[buffer_image_scratch]"
)
{
	std::vector<char> memory(64);
	const auto storage = std::make_shared<mock_buffer>();
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*storage, get_host_ptr()).LR_RETURN(memory.data() + 1);
	ALLOW_CALL(*storage, get_size()).RETURN(32);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_MATCHES(
		buffer_image_scratch(make_span(locations), files, storage),
		std::invalid_argument,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("stack.mrcs: buffer_image_scratch: ")
		)
	);
}

TEST_CASE(
	"a buffer_image_scratch holds no more than its buffer has room for",
	"[buffer_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack_0.mrcs", 0),
		image_location("stack_0.mrcs", 1),
		image_location("stack_1.mrcs", 0),
		image_location("stack_1.mrcs", 1),
		image_location("stack_2.mrcs", 0)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));

	SECTION( "a file that fits whole leaves room for the next" )
	{
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_2.mrcs")).RETURN(reader);

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			make_storage(5 * image_bytes)
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
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({0}) );

		buffer_image_scratch scratch(
			make_span(locations),
			files,
			make_storage(3 * image_bytes + image_bytes / 2)
		);

		REQUIRE( scratch.find("stack_1.mrcs") != nullptr );
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
		scratch.find("stack_1.mrcs")->store(*reader, whole_stack());
	}

	SECTION( "a file none of which fits has no entry" )
	{
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_1.mrcs"));

		const buffer_image_scratch scratch(
			make_span(locations),
			files,
			make_storage(image_bytes - sizeof(float))
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
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 8)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_MATCHES(
		buffer_image_scratch(
			make_span(locations),
			files,
			make_storage(8 * image_bytes)
		),
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
	const std::vector<image_location> locations = {
		image_location("absent.mrcs", 0)
	};

	REQUIRE_CALL(files, acquire("absent.mrcs"))
		.SIDE_EFFECT( throw unsupported_operation_error("nothing claims it") )
		.RETURN(nullptr);

	REQUIRE_THROWS_AS(
		buffer_image_scratch(
			make_span(locations),
			files,
			make_storage(8 * image_bytes)
		),
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
	const std::vector<image_location> locations = {
		image_location(path.get(), 5),
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire(path.get())).RETURN(reader);

	SECTION( "all of them when they fit" )
	{
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			make_storage(3 * image_bytes)
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

	SECTION( "the lowest indices when the buffer cuts the file" )
	{
		buffer_image_scratch scratch(
			make_span(locations),
			files,
			make_storage(2 * image_bytes)
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
			make_storage(3 * image_bytes),
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
			make_storage(3 * image_bytes),
			1
		);
		const auto entry = scratch.find(path.get());

		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({3}));
	}
}

TEST_CASE(
	"the entries of a buffer_image_scratch do not overwrite one another",
	"[buffer_image_scratch]"
)
{
	const scoped_path first_path("buffer_scratch_first.raw");
	const scoped_path second_path("buffer_scratch_second.raw");
	write_counting_image_file(first_path.get(), stack_extents);
	write_counting_image_file(second_path.get(), stack_extents);
	const auto stack =
		open_counting_image_file(first_path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(first_path.get(), 1),
		image_location(first_path.get(), 3),
		image_location(second_path.get(), 5)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(trompeloeil::_)).RETURN(reader);
	ALLOW_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_SIDE_EFFECT( stack->read(_1, _2) );

	buffer_image_scratch scratch(
		make_span(locations),
		files,
		make_storage(3 * image_bytes)
	);
	const auto first = scratch.find(first_path.get());
	const auto second = scratch.find(second_path.get());

	// Both entries are loaded before either is read.
	first->store(*reader, whole_stack());
	second->store(*reader, whole_stack());

	auto pair =
		make_host_array<float>({2, 2, 2}, numerical_type::float32, -1.0F);
	auto single =
		make_host_array<float>({1, 2, 2}, numerical_type::float32, -1.0F);
	first->read(array_ref(pair), images({1, 3}));
	second->read(array_ref(single), images({5}));

	auto expected_pair = count_from(1 * image_size, image_size);
	const auto third = count_from(3 * image_size, image_size);
	expected_pair.insert(expected_pair.end(), third.begin(), third.end());

	CHECK( get_values<float>(pair) == expected_pair );
	CHECK( get_values<float>(single) ==
		count_from(5 * image_size, image_size) );
}
