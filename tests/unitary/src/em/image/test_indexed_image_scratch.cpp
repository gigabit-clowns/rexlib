// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/indexed_image_scratch.hpp>

#include "fixtures/counting_image_file.hpp"
#include "fixtures/scoped_path.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"
#include "mock/mock_image_scratch_storage.hpp"

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/exceptions/unsupported_operation_error.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_location_grouping.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/host_image_scratch_storage.hpp>
#include <rexlib/em/image/image_scratch_entry.hpp>
#include <rexlib/em/image/image_scratch_open_mode.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <boost/filesystem/operations.hpp>
#include <cstddef>
#include <cstdint>
#include <fstream>
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

// Storage of a number of bytes in main memory.
std::shared_ptr<image_scratch_storage> make_storage(std::size_t size)
{
	return create_host_image_scratch_storage(size);
}

// The bytes a scratch takes to hold some images of each of some stacks: its
// fingerprint, then for each stack its flags followed by its images.
std::size_t storage_bytes(
	const std::vector<std::size_t> &image_counts,
	std::size_t flags_per_stack = 1
)
{
	auto bytes = sizeof(std::uint64_t);
	for (const auto image_count : image_counts)
	{
		bytes += flags_per_stack * sizeof(std::uint64_t);
		bytes += image_count * image_bytes;
	}

	return bytes;
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
	"an indexed_image_scratch needs storage it can use",
	"[indexed_image_scratch]"
)
{
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	SECTION( "null storage is refused" )
	{
		REQUIRE_THROWS_AS(
			indexed_image_scratch(
				group(locations),
				files,
				nullptr,
				one_run
			),
			std::invalid_argument
		);
	}

	SECTION( "storage that is not aligned for 64-bit integers is refused" )
	{
		alignas(std::uint64_t) byte memory[64] = {};
		const auto storage = std::make_shared<mock_image_scratch_storage>();
		const mock_image_scratch_storage &const_storage = *storage;
		ALLOW_CALL(*storage, get_data()).LR_RETURN(memory + 1);
		ALLOW_CALL(const_storage, get_data()).LR_RETURN(memory + 1);
		ALLOW_CALL(*storage, get_size()).RETURN(32);

		REQUIRE_THROWS_AS(
			indexed_image_scratch(
				group(locations),
				files,
				storage,
				one_run
			),
			std::invalid_argument
		);
	}

	SECTION( "storage smaller than a fingerprint is refused" )
	{
		alignas(std::uint64_t) byte memory[64] = {};
		const auto storage = std::make_shared<mock_image_scratch_storage>();
		const mock_image_scratch_storage &const_storage = *storage;
		ALLOW_CALL(*storage, get_data()).LR_RETURN(memory);
		ALLOW_CALL(const_storage, get_data()).LR_RETURN(memory);
		ALLOW_CALL(*storage, get_size()).RETURN(4);

		REQUIRE_THROWS_AS(
			indexed_image_scratch(
				group(locations),
				files,
				storage,
				one_run
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"an indexed_image_scratch refuses a run of no index",
	"[indexed_image_scratch]"
)
{
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	REQUIRE_THROWS_AS(
		indexed_image_scratch(
			group(locations),
			files,
			make_storage(8 * image_bytes),
			0
		),
		std::invalid_argument
	);
}

TEST_CASE(
	"an indexed_image_scratch holds an entry for each file that is named",
	"[indexed_image_scratch]"
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

	indexed_image_scratch scratch(
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

	SECTION( "the entries are stored one after another in the storage" )
	{
		// The fingerprint and a flag take four elements, then come two
		// images of four elements; another flag, then one image.
		REQUIRE_CALL(*first, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_first_element(_1) == 4 );
		REQUIRE_CALL(*second, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_first_element(_1) == 4 + 2 * image_size + 2 );

		scratch.find("stack_0.mrcs")->store(*first, whole_stack());
		scratch.find("stack_1.mrcs")->store(*second, whole_stack());
	}
}

TEST_CASE(
	"an indexed_image_scratch holds every index of a file named as a whole",
	"[indexed_image_scratch]"
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

	indexed_image_scratch scratch(
		group(locations),
		files,
		make_storage(storage_bytes({8})),
		one_run
	);

	scratch.find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"an indexed_image_scratch aligns each entry for the data type of its file",
	"[indexed_image_scratch]"
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

	indexed_image_scratch scratch(
		group(locations),
		files,
		make_storage(64),
		one_run
	);

	// The fingerprint, a flag and nine bytes are taken. The flag of the
	// stack starts at the next multiple of eight, byte 32, and its float32
	// values at byte 40: their tenth element.
	REQUIRE_CALL(*stack, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_first_element(_1) == 10 );

	scratch.find("stack.mrcs")->store(*stack, whole_stack());
}

TEST_CASE(
	"an indexed_image_scratch refuses storage that is not aligned for a file",
	"[indexed_image_scratch]"
)
{
	// A stack whose elements take sixteen bytes, and storage that is
	// aligned for eight and not for sixteen.
	const image_descriptor complex_descriptor(
		make_span(stack_extents),
		2,
		numerical_type::complex_float64
	);
	alignas(16) byte memory[512] = {};
	const auto storage = std::make_shared<mock_image_scratch_storage>();
	const mock_image_scratch_storage &const_storage = *storage;
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 0)
	};

	ALLOW_CALL(*storage, get_data()).LR_RETURN(memory + 8);
	ALLOW_CALL(const_storage, get_data()).LR_RETURN(memory + 8);
	ALLOW_CALL(*storage, get_size()).RETURN(256);
	ALLOW_CALL(*reader, get_descriptor())
		.RETURN(std::ref(complex_descriptor));
	REQUIRE_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	REQUIRE_THROWS_MATCHES(
		indexed_image_scratch(group(locations), files, storage, one_run),
		std::invalid_argument,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("stack.mrcs: indexed_image_scratch: ")
		)
	);
}

TEST_CASE(
	"an indexed_image_scratch holds no more than its storage has room for",
	"[indexed_image_scratch]"
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

		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({2, 2, 1})),
			one_run
		);

		CHECK( scratch.find("stack_2.mrcs") != nullptr );
	}

	SECTION( "the first file that does not fit is cut, and none follows" )
	{
		// Room for two images of the first stack, and for one and a half of
		// the second. The third stack is not opened.
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		REQUIRE_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_2.mrcs"));
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({0}) );

		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({2, 1}) + image_bytes / 2),
			one_run
		);

		REQUIRE( scratch.find("stack_1.mrcs") != nullptr );
		CHECK( scratch.find("stack_2.mrcs") == nullptr );
		scratch.find("stack_1.mrcs")->store(*reader, whole_stack());
	}

	SECTION( "a file that follows one that fills the storage has no entry" )
	{
		// Room for the two images of the first stack and no more.
		REQUIRE_CALL(files, acquire("stack_0.mrcs")).RETURN(reader);
		ALLOW_CALL(files, acquire("stack_1.mrcs")).RETURN(reader);
		FORBID_CALL(files, acquire("stack_2.mrcs"));

		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({2})),
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

		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({0})),
			one_run
		);

		CHECK( scratch.find("stack_0.mrcs") == nullptr );
	}
}

TEST_CASE(
	"an indexed_image_scratch refuses a stack index that a file does not have",
	"[indexed_image_scratch]"
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
		indexed_image_scratch(
			group(locations),
			files,
			make_storage(8 * image_bytes),
			one_run
		),
		std::out_of_range,
		Catch::Matchers::MessageMatches(
			Catch::Matchers::StartsWith("stack.mrcs: indexed_image_scratch: ")
		)
	);
}

TEST_CASE(
	"an indexed_image_scratch reports what opening a file reported",
	"[indexed_image_scratch]"
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
		indexed_image_scratch(
			group(locations),
			files,
			make_storage(8 * image_bytes),
			one_run
		),
		unsupported_operation_error
	);
}

TEST_CASE(
	"an entry of an indexed_image_scratch serves the images it holds",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("indexed_scratch_serves.raw");
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
		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({3})),
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

	SECTION( "the lowest indices when the storage cuts the file" )
	{
		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({2})),
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
		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({3}, 2)),
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
		indexed_image_scratch scratch(
			group(locations),
			files,
			make_storage(storage_bytes({3}, 3)),
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
	"the entries of an indexed_image_scratch do not overwrite one another",
	"[indexed_image_scratch]"
)
{
	const scoped_path first_path("indexed_scratch_first.raw");
	const scoped_path second_path("indexed_scratch_second.raw");
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

	indexed_image_scratch scratch(
		group(locations),
		files,
		make_storage(storage_bytes({2, 1})),
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
	"[indexed_image_scratch]"
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
	"[indexed_image_scratch]"
)
{
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs")
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	const auto scratch = create_host_image_scratch(
		group(locations),
		files,
		one_run,
		storage_bytes({3}) + image_bytes / 2
	);

	// Three of the eight images fit.
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({0, 1, 2}) );

	scratch->find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"create_host_image_scratch refuses to hold nothing",
	"[indexed_image_scratch]"
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
	"[indexed_image_scratch]"
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
			storage_bytes({3}) );
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
	"[indexed_image_scratch]"
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

	// The fingerprint, a flag and nine bytes; seven of padding, a flag and
	// the sixteen bytes of the image.
	CHECK( boost::filesystem::file_size(storage_path.get()) == 56 );

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
	"[indexed_image_scratch]"
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
	const auto max_size = storage_bytes({2}) + sizeof(float);

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
		storage_bytes({2}) );

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({1, 3}) );

	scratch->find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"create_mapped_file_image_scratch creates no file when it refuses to "
	"hold nothing",
	"[indexed_image_scratch]"
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
	"[indexed_image_scratch]"
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
		image_file_error
	);
}

TEST_CASE(
	"a resumed indexed_image_scratch keeps what an earlier one loaded into "
	"its storage",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("indexed_scratch_resumed.raw");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 5),
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	// Runs of two images: indices 1 and 3 are one run, 5 another.
	const auto storage = make_storage(storage_bytes({3}, 2));

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(path.get())).RETURN(reader);

	// An earlier scratch loads the first run.
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		indexed_image_scratch earlier(group(locations), files, storage, 2);
		earlier.find(path.get())->store(*reader, images({1}));
	}

	SECTION( "a run that was loaded is not loaded again" )
	{
		FORBID_CALL(*reader, read(trompeloeil::_, trompeloeil::_));

		indexed_image_scratch scratch(
			group(locations),
			files,
			storage,
			2,
			image_scratch_open_mode::resumed
		);
		const auto entry = scratch.find(path.get());
		auto destination = make_host_array<float>(
			{2, 2, 2},
			numerical_type::float32,
			-1.0F
		);

		entry->store(*reader, images({3, 1}));
		const auto missing =
			entry->read(array_ref(destination), images({3, 1}));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({3, 1}) );
	}

	SECTION( "a run that was not loaded is loaded" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({5}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		indexed_image_scratch scratch(
			group(locations),
			files,
			storage,
			2,
			image_scratch_open_mode::resumed
		);

		scratch.find(path.get())->store(*reader, images({5}));
	}

	SECTION( "a scratch that starts empty keeps nothing" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		indexed_image_scratch scratch(group(locations), files, storage, 2);

		scratch.find(path.get())->store(*reader, images({1}));
	}

	SECTION( "a scratch of another layout keeps nothing" )
	{
		// With runs of four the three images are one run, whose flag is
		// where the flag of the loaded run was.
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3, 5}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		indexed_image_scratch scratch(
			group(locations),
			files,
			storage,
			4,
			image_scratch_open_mode::resumed
		);

		scratch.find(path.get())->store(*reader, images({1}));
	}

	SECTION( "a run is loaded again when its file was modified since" )
	{
		boost::filesystem::last_write_time(
			path.get(),
			boost::filesystem::last_write_time(path.get()) + 10
		);
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 3}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		indexed_image_scratch scratch(
			group(locations),
			files,
			storage,
			2,
			image_scratch_open_mode::resumed
		);

		scratch.find(path.get())->store(*reader, images({1}));
	}
}

TEST_CASE(
	"a resumed indexed_image_scratch keeps the files that were not modified",
	"[indexed_image_scratch]"
)
{
	const scoped_path first_path("indexed_scratch_resumed_first.raw");
	const scoped_path second_path("indexed_scratch_resumed_second.raw");
	write_counting_image_file(first_path.get(), stack_extents);
	write_counting_image_file(second_path.get(), stack_extents);

	const auto first = std::make_shared<mock_image_reader>();
	const auto second = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(first_path.get(), 1),
		image_location(second_path.get(), 5)
	};
	const auto storage = make_storage(storage_bytes({1, 1}));

	ALLOW_CALL(*first, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*second, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(first_path.get())).RETURN(first);
	ALLOW_CALL(files, acquire(second_path.get())).RETURN(second);

	// An earlier scratch loads both files.
	{
		REQUIRE_CALL(*first, read(trompeloeil::_, trompeloeil::_));
		REQUIRE_CALL(*second, read(trompeloeil::_, trompeloeil::_));

		indexed_image_scratch earlier(
			group(locations),
			files,
			storage,
			one_run
		);
		earlier.find(first_path.get())->store(*first, whole_stack());
		earlier.find(second_path.get())->store(*second, whole_stack());
	}

	// Only the second file is modified, and only it is loaded again.
	boost::filesystem::last_write_time(
		second_path.get(),
		boost::filesystem::last_write_time(second_path.get()) + 10
	);
	FORBID_CALL(*first, read(trompeloeil::_, trompeloeil::_));
	REQUIRE_CALL(*second, read(trompeloeil::_, trompeloeil::_));

	indexed_image_scratch scratch(
		group(locations),
		files,
		storage,
		one_run,
		image_scratch_open_mode::resumed
	);

	scratch.find(first_path.get())->store(*first, whole_stack());
	scratch.find(second_path.get())->store(*second, whole_stack());
}

TEST_CASE(
	"a resumed indexed_image_scratch keeps nothing of a file whose "
	"modification time can not be read",
	"[indexed_image_scratch]"
)
{
	// There is no file at this path, only a reader that the provider makes.
	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 1)
	};
	const auto storage = make_storage(storage_bytes({1}));

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire("stack.mrcs")).RETURN(reader);

	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_));

		indexed_image_scratch earlier(
			group(locations),
			files,
			storage,
			one_run
		);
		earlier.find("stack.mrcs")->store(*reader, whole_stack());
	}

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_));

	indexed_image_scratch scratch(
		group(locations),
		files,
		storage,
		one_run,
		image_scratch_open_mode::resumed
	);

	scratch.find("stack.mrcs")->store(*reader, whole_stack());
}

TEST_CASE(
	"create_mapped_file_image_scratch resumes from the file that an earlier "
	"scratch left",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("mapped_file_image_scratch_reuse.raw");
	const scoped_path storage_path("mapped_file_image_scratch_reuse.scratch");
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
	ALLOW_CALL(files, acquire(path.get())).RETURN(reader);

	// An earlier scratch loads the first of its runs of two: images 1 and 3.
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		const auto earlier = create_mapped_file_image_scratch(
			group(locations),
			files,
			storage_path.get(),
			2
		);
		earlier->find(path.get())->store(*reader, images({1}));
	}

	SECTION( "a run that was loaded is served without its file being read" )
	{
		FORBID_CALL(*reader, read(trompeloeil::_, trompeloeil::_));

		const auto scratch = create_mapped_file_image_scratch(
			group(locations),
			files,
			storage_path.get(),
			2
		);
		const auto entry = scratch->find(path.get());
		auto destination = make_host_array<float>(
			{2, 2, 2},
			numerical_type::float32,
			-1.0F
		);

		entry->store(*reader, images({3, 1}));
		const auto missing =
			entry->read(array_ref(destination), images({3, 1}));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({3, 1}) );
	}

	SECTION( "a run that was not loaded is loaded" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({5}) )
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		const auto scratch = create_mapped_file_image_scratch(
			group(locations),
			files,
			storage_path.get(),
			2
		);

		scratch->find(path.get())->store(*reader, images({5}));
	}

}

TEST_CASE(
	"create_mapped_file_image_scratch overwrites a file that holds another "
	"scratch",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("mapped_file_image_scratch_other.raw");
	const scoped_path storage_path("mapped_file_image_scratch_other.scratch");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	// As many images of the same stack, so that the file is as large.
	const std::vector<image_location> others = {
		image_location(path.get(), 1),
		image_location(path.get(), 5)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(path.get())).RETURN(reader);

	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		const auto earlier = create_mapped_file_image_scratch(
			group(locations),
			files,
			storage_path.get(),
			one_run
		);
		earlier->find(path.get())->store(*reader, whole_stack());
	}

	// Nothing of the earlier scratch is kept: the run is loaded.
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({1, 5}) )
		.LR_SIDE_EFFECT( stack->read(_1, _2) );

	const auto scratch = create_mapped_file_image_scratch(
		group(others),
		files,
		storage_path.get(),
		one_run
	);
	const auto entry = scratch->find(path.get());
	auto destination =
		make_host_array<float>({2, 2, 2}, numerical_type::float32, -1.0F);

	entry->store(*reader, whole_stack());
	const auto missing = entry->read(array_ref(destination), images({5, 1}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({5, 1}) );
	CHECK( boost::filesystem::file_size(storage_path.get()) ==
		storage_bytes({2}) );
}

TEST_CASE(
	"create_mapped_file_image_scratch overwrites a file of another size",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("mapped_file_image_scratch_resized.raw");
	const scoped_path storage_path(
		"mapped_file_image_scratch_resized.scratch"
	);
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 1),
		image_location(path.get(), 3),
		image_location(path.get(), 5)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(path.get())).RETURN(reader);
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({1, 3, 5}) )
		.LR_SIDE_EFFECT( stack->read(_1, _2) );

	SECTION( "a file that is not a scratch" )
	{
		std::ofstream output(
			storage_path.get().c_str(),
			std::ios::out | std::ios::binary
		);
		output << "what was at the path";
	}

	SECTION( "a scratch of fewer images" )
	{
		const std::vector<image_location> fewer = {
			image_location(path.get(), 1)
		};
		const auto earlier = create_mapped_file_image_scratch(
			group(fewer),
			files,
			storage_path.get(),
			one_run
		);
	}

	const auto scratch = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run
	);
	const auto entry = scratch->find(path.get());
	auto destination =
		make_host_array<float>({3, 2, 2}, numerical_type::float32, -1.0F);

	entry->store(*reader, whole_stack());
	const auto missing =
		entry->read(array_ref(destination), images({5, 1, 3}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({5, 1, 3}) );
	CHECK( boost::filesystem::file_size(storage_path.get()) ==
		storage_bytes({3}) );
}

TEST_CASE(
	"scratches that hold the same images share one file while both are "
	"alive",
	"[indexed_image_scratch]"
)
{
	const scoped_path path("mapped_file_image_scratch_shared.raw");
	const scoped_path storage_path("mapped_file_image_scratch_shared.scratch");
	write_counting_image_file(path.get(), stack_extents);
	const auto stack = open_counting_image_file(path.get(), stack_extents, 2);

	const auto reader = std::make_shared<mock_image_reader>();
	mock_image_reader_provider files;
	const std::vector<image_location> locations = {
		image_location(path.get(), 1),
		image_location(path.get(), 3)
	};

	ALLOW_CALL(*reader, get_descriptor()).RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(files, acquire(path.get())).RETURN(reader);

	// The first scratch loads its run, and stays alive.
	const auto first = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run
	);
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_SIDE_EFFECT( stack->read(_1, _2) );

		first->find(path.get())->store(*reader, whole_stack());
	}

	// The second finds the run loaded.
	FORBID_CALL(*reader, read(trompeloeil::_, trompeloeil::_));

	const auto second = create_mapped_file_image_scratch(
		group(locations),
		files,
		storage_path.get(),
		one_run
	);
	const auto entry = second->find(path.get());
	auto destination =
		make_host_array<float>({2, 2, 2}, numerical_type::float32, -1.0F);

	entry->store(*reader, whole_stack());
	const auto missing = entry->read(array_ref(destination), images({3, 1}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({3, 1}) );
}
