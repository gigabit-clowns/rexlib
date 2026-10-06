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
#include <rexlib/core/exceptions/file_error.hpp>
#include <rexlib/core/exceptions/unsupported_capability_error.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_location_grouping.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <boost/filesystem/operations.hpp>
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

// Long enough for one run to span everything that is held of a stack.
const std::size_t one_run = 8;

using index_list = std::vector<std::size_t>;

image_location_grouping group(const std::vector<image_location> &locations)
{
	return image_location_grouping(make_span(locations));
}

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

// A plan of one region that spans a whole file of some extents.
image_transfer_plan whole_file(const std::vector<std::size_t> &extents)
{
	const auto rank = extents.size();
	image_transfer_plan plan(image_transfer_shape(extents, rank, rank));
	const std::vector<std::size_t> offset(rank, 0);
	plan.add(make_span(offset), make_span(offset));

	return plan;
}

// The values of some images of a counting stack, one after another.
std::vector<float> values_of_images(const std::vector<std::size_t> &indices)
{
	std::vector<float> values;
	for (const auto index : indices)
	{
		const auto image = count_from(index * image_size, image_size);
		values.insert(values.end(), image.begin(), image.end());
	}

	return values;
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
			buffer_image_scratch(
				group(locations),
				files,
				nullptr,
				one_run
			),
			std::invalid_argument
		);
	}

	SECTION( "a buffer the host can not reach is refused" )
	{
		const auto storage = std::make_shared<mock_buffer>();
		const mock_buffer &const_storage = *storage;
		ALLOW_CALL(*storage, get_host_ptr()).RETURN(nullptr);
		ALLOW_CALL(const_storage, get_host_ptr()).RETURN(nullptr);

		REQUIRE_THROWS_AS(
			buffer_image_scratch(
				group(locations),
				files,
				storage,
				one_run
			),
			unsupported_capability_error
		);
	}
}

TEST_CASE(
	"a buffer_image_scratch refuses a run of no index",
	"[buffer_image_scratch]"
)
{
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	REQUIRE_THROWS_AS(
		buffer_image_scratch(
			group(locations),
			files,
			make_storage(8 * image_bytes),
			0
		),
		std::invalid_argument
	);
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
		group(locations),
		files,
		make_storage(8 * image_bytes),
		one_run
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
		group(locations),
		files,
		make_storage(8 * image_bytes),
		one_run
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
		group(locations),
		files,
		make_storage(64),
		one_run
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
	const mock_buffer &const_storage = *storage;
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*storage, get_host_ptr()).LR_RETURN(memory.data() + 1);
	ALLOW_CALL(const_storage, get_host_ptr()).LR_RETURN(memory.data() + 1);
	ALLOW_CALL(*storage, get_size()).RETURN(32);
	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_MATCHES(
		buffer_image_scratch(group(locations), files, storage, one_run),
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

		buffer_image_scratch scratch(
			group(locations),
			files,
			make_storage(5 * image_bytes),
			one_run
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
			group(locations),
			files,
			make_storage(3 * image_bytes + image_bytes / 2),
			one_run
		);

		REQUIRE( scratch.find("stack_1.mrcs") != nullptr );
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
		scratch.find("stack_1.mrcs")->store(*reader, whole_stack());
	}

	SECTION( "a file that follows one that fills the buffer has no entry" )
	{
		// Room for the two images of the first stack and no more.
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		ALLOW_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_2.mrcs"));

		buffer_image_scratch scratch(
			group(locations),
			files,
			make_storage(2 * image_bytes),
			one_run
		);

		CHECK( scratch.find("stack_0.mrcs") != nullptr );
		CHECK( scratch.find("stack_1.mrcs") == nullptr );
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
	}

	SECTION( "a file none of which fits has no entry" )
	{
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_1.mrcs"));

		buffer_image_scratch scratch(
			group(locations),
			files,
			make_storage(image_bytes - sizeof(float)),
			one_run
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
			group(locations),
			files,
			make_storage(8 * image_bytes),
			one_run
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
			group(locations),
			files,
			make_storage(8 * image_bytes),
			one_run
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
			group(locations),
			files,
			make_storage(3 * image_bytes),
			one_run
		);
		const auto entry = scratch.find(path.get());
		auto destination = make_host_array<float>(
			{3, 2, 2},
			numerical_type::float32,
			-1.0F
		);

		// One run: one read of the file brings in all three.
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
			group(locations),
			files,
			make_storage(2 * image_bytes),
			one_run
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

	SECTION( "a run at a time when it holds more of them than a run has" )
	{
		// Runs of two images: indices 1 and 3 are one run, 5 another.
		buffer_image_scratch scratch(
			group(locations),
			files,
			make_storage(3 * image_bytes),
			2
		);
		const auto entry = scratch.find(path.get());

		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		entry->store(*reader, images({3}));
	}

	SECTION( "one at a time when a run has one index" )
	{
		buffer_image_scratch scratch(
			group(locations),
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
		group(locations),
		files,
		make_storage(3 * image_bytes),
		one_run
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

TEST_CASE(
	"create_host_image_scratch holds the images that the locations name",
	"[buffer_image_scratch]"
)
{
	const scoped_path path("host_image_scratch.raw");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 5),
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	// The file is opened to compute the size, and again to hold its images.
	REQUIRE_CALL(files, acquire(path.get())).TIMES(2).RETURN(stack);

	const auto scratch =
		create_host_image_scratch(group(locations), files, one_run);

	REQUIRE( scratch != nullptr );
	const auto entry = scratch->find(path.get());
	REQUIRE( entry != nullptr );

	auto destination =
		make_host_array<float>({3, 2, 2}, numerical_type::float32, -1.0F);
	entry->store(*stack, images({5, 1, 3}));
	const auto missing =
		entry->read(array_ref(destination), images({5, 1, 3}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({5, 1, 3}) );
}

TEST_CASE(
	"create_host_image_scratch allocates no more than its maximum size",
	"[buffer_image_scratch]"
)
{
	// Images as large as the alignment of the allocation, so that the
	// allocator rounds no size up.
	const auto allocator = get_host_memory_resource().create_allocator();
	const auto large_image_bytes = allocator->get_max_alignment();
	const std::vector<std::size_t> extents = {
		8, 2, large_image_bytes / (2 * sizeof(float))
	};
	const image_descriptor descriptor(
		make_span(extents),
		2,
		numerical_type::float32
	);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs")
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(descriptor));
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	const auto scratch = create_host_image_scratch(
		group(locations),
		files,
		one_run,
		3 * large_image_bytes
	);

	// Three of the eight images fit.
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({0, 1, 2}) );

	scratch->find("stack.mrcs")->store(*reader, whole_file(extents));
}

TEST_CASE(
	"create_host_image_scratch refuses to hold nothing",
	"[buffer_image_scratch]"
)
{
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	SECTION( "when no location is given" )
	{
		REQUIRE_THROWS_AS(
			create_host_image_scratch(group({}), files, one_run),
			std::invalid_argument
		);
	}

	SECTION( "when the maximum size has no room for an image" )
	{
		const auto reader = std::make_shared<mock_image_reader>();
		ALLOW_CALL(*reader, get_descriptor())
			.RETURN(std::ref(stack_descriptor));
		ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

		REQUIRE_THROWS_AS(
			create_host_image_scratch(
				group(locations),
				files,
				one_run,
				image_bytes - 1
			),
			std::invalid_argument
		);
	}

	SECTION( "when a run has no index" )
	{
		REQUIRE_THROWS_AS(
			create_host_image_scratch(group(locations), files, 0),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"create_mapped_file_image_scratch stores the images in a file of the "
	"size they need",
	"[buffer_image_scratch]"
)
{
	const scoped_path path("mapped_file_image_scratch.raw");
	const scoped_path storage_path("mapped_file_image_scratch.scratch");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 5),
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	REQUIRE_CALL(files, acquire(path.get())).TIMES(2).RETURN(stack);

	const auto scratch = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run
	);

	REQUIRE( scratch != nullptr );
	const auto entry = scratch->find(path.get());
	REQUIRE( entry != nullptr );

	SECTION( "the file has room for the three images and no more" )
	{
		CHECK( boost::filesystem::file_size(storage_path.get()) ==
			3 * image_bytes );
	}

	SECTION( "the images are read back from it" )
	{
		auto destination =
			make_host_array<float>({3, 2, 2}, numerical_type::float32, -1.0F);
		entry->store(*stack, images({5, 1, 3}));
		const auto missing =
			entry->read(array_ref(destination), images({5, 1, 3}));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) ==
			values_of_images({5, 1, 3}) );
	}
}

TEST_CASE(
	"create_mapped_file_image_scratch leaves room between files of "
	"different data types",
	"[buffer_image_scratch]"
)
{
	// Three rows of three bytes, then one image of a float32 stack.
	const std::vector<std::size_t> bytes_extents = {3, 3};
	const image_descriptor bytes_descriptor(
		make_span(bytes_extents),
		2,
		numerical_type::uint8
	);
	const scoped_path storage_path("mapped_file_image_scratch_types.scratch");
	const auto bytes = std::make_shared<mock_image_reader>();
	const auto stack = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("bytes.mrc"),
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*bytes, get_descriptor()).RETURN(std::ref(bytes_descriptor));
	ALLOW_CALL(*stack, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire("bytes.mrc")).RETURN(bytes);
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(stack);

	const auto scratch = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run
	);

	// Nine bytes, three of padding, and the sixteen of the image.
	CHECK( boost::filesystem::file_size(storage_path.get()) == 28 );

	// The image is held whole, so the file is as large as its placing
	// needs.
	REQUIRE_CALL(*stack, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({0}) );

	REQUIRE( scratch->find("stack.mrcs") != nullptr );
	scratch->find("stack.mrcs")->store(*stack, whole_stack());
}

TEST_CASE(
	"create_mapped_file_image_scratch creates a file no larger than its "
	"maximum size",
	"[buffer_image_scratch]"
)
{
	const scoped_path storage_path("mapped_file_image_scratch_cut.scratch");
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 5),
		image_location("stack.mrcs", 1),
		image_location("stack.mrcs", 3)
	};
	const auto max_size = 2 * image_bytes + sizeof(float);

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	const auto scratch = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run,
		max_size
	);

	// Two of the three images fit, and the file has room for no more.
	CHECK( boost::filesystem::file_size(storage_path.get()) ==
		2 * image_bytes );

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({1, 3}) );

	scratch->find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"create_mapped_file_image_scratch creates no file when it refuses to "
	"hold nothing",
	"[buffer_image_scratch]"
)
{
	const scoped_path storage_path("mapped_file_image_scratch_none.scratch");
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	SECTION( "when no location is given" )
	{
		REQUIRE_THROWS_AS(
			create_mapped_file_image_scratch(
				group({}),
				files,
				storage_path.get(),
				one_run
			),
			std::invalid_argument
		);
	}

	SECTION( "when the maximum size has no room for an image" )
	{
		const auto reader = std::make_shared<mock_image_reader>();
		ALLOW_CALL(*reader, get_descriptor())
			.RETURN(std::ref(stack_descriptor));
		ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

		REQUIRE_THROWS_AS(
			create_mapped_file_image_scratch(
				group(locations),
				files,
				storage_path.get(),
				one_run,
				image_bytes - 1
			),
			std::invalid_argument
		);
	}

	SECTION( "when a run has no index" )
	{
		REQUIRE_THROWS_AS(
			create_mapped_file_image_scratch(
				group(locations),
				files,
				storage_path.get(),
				0
			),
			std::invalid_argument
		);
	}

	CHECK_FALSE( boost::filesystem::exists(storage_path.get()) );
}

TEST_CASE(
	"create_mapped_file_image_scratch reports a file it can not create",
	"[buffer_image_scratch]"
)
{
	const scoped_path directory("mapped_file_image_scratch_missing");
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_AS(
		create_mapped_file_image_scratch(
			group(locations),
			files,
			directory.get() + "/scratch.bin",
			one_run
		),
		file_error
	);
}
