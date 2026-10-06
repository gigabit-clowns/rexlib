// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <em/image/buffer_image_scratch_entry.hpp>

#include "fixtures/counting_image_file.hpp"
#include "fixtures/scoped_path.hpp"
#include "mock/mock_image_reader.hpp"

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_reader.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <trompeloeil.hpp>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::test;

namespace
{

// A stack of eight images of 2 by 2. Image n holds 4n, 4n + 1, 4n + 2 and
// 4n + 3.
const std::vector<std::size_t> file_extents = {8, 2, 2};
const std::vector<std::size_t> image_extents = {2, 2};
const std::size_t image_size = 4;

const float untouched = -1.0F;

// The stack on disk, and a reader over it.
class stack_file
{
public:
	explicit stack_file(const std::string &name)
		: m_path(name)
	{
		write_counting_image_file(m_path.get(), file_extents);
		m_reader = open_counting_image_file(m_path.get(), file_extents, 2);
	}

	const image_reader& get_reader() const noexcept
	{
		return *m_reader;
	}

private:
	scoped_path m_path;
	std::shared_ptr<const image_reader> m_reader;
};

// An entry that holds some images of the stack, with nothing loaded.
std::shared_ptr<buffer_image_scratch_entry> make_entry(
	std::vector<std::size_t> indices,
	std::size_t run_length
)
{
	const std::vector<std::size_t> extents = {indices.size(), 2, 2};

	return std::make_shared<buffer_image_scratch_entry>(
		std::move(indices),
		make_host_array<float>(extents, numerical_type::float32, 0.0F),
		run_length
	);
}

// A batch of `count` images, every element set to `untouched`.
array make_batch(std::size_t count)
{
	return make_host_array<float>(
		{count, 2, 2},
		numerical_type::float32,
		untouched
	);
}

// Image `indices[i]` of the stack, landing in slot `i` of a batch.
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

// `count` consecutive images from `first`, as one region at the start of a
// batch.
image_transfer_plan image_range(std::size_t first, std::size_t count)
{
	image_transfer_plan plan(image_transfer_shape({count, 2, 2}, 3, 3));
	const std::size_t file_offset[3] = {first, 0, 0};
	const std::size_t array_offset[3] = {0, 0, 0};
	plan.add(make_span(file_offset, 3), make_span(array_offset, 3));

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

std::vector<std::size_t> get_array_slots(const image_transfer_plan &plan)
{
	std::vector<std::size_t> slots;
	for (std::size_t region = 0; region < plan.get_region_count(); ++region)
	{
		slots.push_back(plan.get_array_offset(region).front());
	}

	return slots;
}

// The values of a batch that holds the given images of the stack in order.
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

using index_list = std::vector<std::size_t>;

} // anonymous namespace

TEST_CASE(
	"a buffer_image_scratch_entry needs indices that are strictly ascending",
	"[buffer_image_scratch_entry]"
)
{
	SECTION( "indices that descend are refused" )
	{
		REQUIRE_THROWS_AS(
			buffer_image_scratch_entry(index_list({4, 1, 5}), make_batch(3), 1),
			std::invalid_argument
		);
	}

	SECTION( "an index given twice is refused" )
	{
		REQUIRE_THROWS_AS(
			buffer_image_scratch_entry(index_list({1, 4, 4}), make_batch(3), 1),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry needs values that match its indices",
	"[buffer_image_scratch_entry]"
)
{
	SECTION( "values of another number of indices are refused" )
	{
		REQUIRE_THROWS_AS(
			buffer_image_scratch_entry(index_list({1, 4, 5}), make_batch(2), 1),
			std::invalid_argument
		);
	}

	SECTION( "values that are not initialized are refused" )
	{
		REQUIRE_THROWS_AS(
			buffer_image_scratch_entry(index_list({1, 4, 5}), array(), 1),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry refuses a run of no slot",
	"[buffer_image_scratch_entry]"
)
{
	REQUIRE_THROWS_AS(
		buffer_image_scratch_entry(index_list({1, 4, 5}), make_batch(3), 0),
		std::invalid_argument
	);
}

TEST_CASE(
	"a buffer_image_scratch_entry reads nothing before it is loaded",
	"[buffer_image_scratch_entry]"
)
{
	const auto entry = make_entry({0, 1, 2, 3, 4, 5, 6, 7}, 4);
	auto destination = make_batch(2);

	const auto missing = entry->read(array_ref(destination), images({3, 5}));

	CHECK( get_file_indices(missing) == index_list({3, 5}) );
	CHECK( get_array_slots(missing) == index_list({0, 1}) );
	CHECK( get_values<float>(destination) ==
		std::vector<float>(2 * image_size, untouched) );
}

TEST_CASE(
	"a buffer_image_scratch_entry loads the run of a stored image",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_run.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3, 4, 5, 6, 7}, 4);

	// One read of the file, for the four images of the second run.
	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({4, 5, 6, 7}) )
		.LR_WITH( get_array_slots(_2) == index_list({4, 5, 6, 7}) )
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

	entry->store(file, images({5}));

	SECTION( "the stored image is read from the entry" )
	{
		auto destination = make_batch(1);

		const auto missing = entry->read(array_ref(destination), images({5}));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({5}) );
	}

	SECTION( "so are the other images of its run" )
	{
		auto destination = make_batch(2);

		const auto missing =
			entry->read(array_ref(destination), images({7, 4}));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({7, 4}) );
	}

	SECTION( "the images of other runs are still missing" )
	{
		auto destination = make_batch(2);

		const auto missing =
			entry->read(array_ref(destination), images({1, 6}));

		CHECK( get_file_indices(missing) == index_list({1}) );
		CHECK( get_array_slots(missing) == index_list({0}) );
	}

	SECTION( "storing an image of the same run does not read the file" )
	{
		entry->store(file, images({6, 4}));
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry splits its images into runs of one length",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_runs.raw");
	mock_image_reader file;

	// Eight images in runs of three: two whole runs and one of what is left.
	const auto entry = make_entry({0, 1, 2, 3, 4, 5, 6, 7}, 3);

	SECTION( "the last run has the images that are left" )
	{
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({6, 7}) )
			.LR_WITH( get_array_slots(_2) == index_list({6, 7}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

		entry->store(file, images({7}));
	}

	SECTION( "a region across two runs loads both" )
	{
		trompeloeil::sequence order;
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_WITH( get_file_indices(_2) == index_list({0, 1, 2}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_WITH( get_file_indices(_2) == index_list({3, 4, 5}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

		entry->store(file, image_range(2, 2));
	}

	SECTION( "a region across two runs is missing until both are loaded" )
	{
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.TIMES(2)
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		auto destination = make_batch(2);

		entry->store(file, images({2}));
		CHECK( entry->read(array_ref(destination), image_range(2, 2))
			.get_region_count() == 1 );

		entry->store(file, images({3}));
		CHECK( entry->read(array_ref(destination), image_range(2, 2))
			.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({2, 3}) );
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry loads all its images at once when a run is "
	"long enough",
	"[buffer_image_scratch_entry]"
)
{
	const auto run_length = GENERATE(
		std::size_t(4),
		std::size_t(9),
		std::numeric_limits<std::size_t>::max()
	);
	const stack_file stack("buffer_scratch_entry_one_run.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3}, run_length);

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH( get_file_indices(_2) == index_list({0, 1, 2, 3}) )
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

	entry->store(file, images({2}));
}

TEST_CASE(
	"a buffer_image_scratch_entry loads only the images it holds",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_subset.raw");
	mock_image_reader file;
	const auto entry = make_entry({1, 4, 5}, 3);

	SECTION( "storing a held image loads the held images of its run" )
	{
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 4, 5}) )
			.LR_WITH( get_array_slots(_2) == index_list({0, 1, 2}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		auto destination = make_batch(3);

		entry->store(file, images({4}));
		const auto missing =
			entry->read(array_ref(destination), images({4, 2, 1}));

		// Image 2 is not held: it stays missing and its slot untouched.
		auto expected = values_of_images({4, 0, 1});
		std::fill_n(expected.begin() + image_size, image_size, untouched);

		CHECK( get_file_indices(missing) == index_list({2}) );
		CHECK( get_array_slots(missing) == index_list({1}) );
		CHECK( get_values<float>(destination) == expected );
	}

	SECTION( "storing a region loads the held images among the ones it spans" )
	{
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH( get_file_indices(_2) == index_list({1, 4, 5}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		auto destination = make_batch(6);

		entry->store(file, image_range(0, 6));
		const auto missing =
			entry->read(array_ref(destination), image_range(0, 6));

		// Images 0, 2 and 3 are not held, so the region is not read.
		CHECK( get_file_indices(missing) == index_list({0}) );
		CHECK( get_values<float>(destination) ==
			std::vector<float>(6 * image_size, untouched) );
	}

	SECTION( "storing images that are not held reads nothing" )
	{
		FORBID_CALL(file, read(trompeloeil::_, trompeloeil::_));

		entry->store(file, images({0, 2, 3, 7}));
	}

	SECTION( "storing a region between held images reads nothing" )
	{
		FORBID_CALL(file, read(trompeloeil::_, trompeloeil::_));

		entry->store(file, image_range(2, 2));
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry takes a region that reaches past the "
	"largest index there is",
	"[buffer_image_scratch_entry]"
)
{
	const std::size_t max_index = std::numeric_limits<std::size_t>::max();
	const stack_file stack("buffer_scratch_entry_overflow.raw");
	mock_image_reader file;
	const auto entry = make_entry({1, 4, 5}, 1);

	SECTION( "storing it loads the images held from its first index on" )
	{
		trompeloeil::sequence order;
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_WITH( get_file_indices(_2) == index_list({4}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_WITH( get_file_indices(_2) == index_list({5}) )
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

		entry->store(file, image_range(4, max_index));
	}

	SECTION( "storing it reads nothing when none of them is held" )
	{
		FORBID_CALL(file, read(trompeloeil::_, trompeloeil::_));

		entry->store(file, image_range(max_index, max_index));
	}

	SECTION( "reading it answers it" )
	{
		auto destination = make_batch(2);

		const auto missing =
			entry->read(array_ref(destination), image_range(4, max_index));

		CHECK( get_file_indices(missing) == index_list({4}) );
		CHECK( get_values<float>(destination) ==
			std::vector<float>(2 * image_size, untouched) );
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry reads a region that spans several images",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_range.raw");
	mock_image_reader file;
	const auto entry = make_entry({2, 3, 4}, 3);

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
	auto destination = make_batch(2);

	SECTION( "when all of them are held" )
	{
		entry->store(file, image_range(3, 2));
		const auto missing =
			entry->read(array_ref(destination), image_range(3, 2));

		CHECK( missing.get_region_count() == 0 );
		CHECK( get_values<float>(destination) == values_of_images({3, 4}) );
	}

	SECTION( "and not when the last of them is not" )
	{
		// The held part is still loaded: images 4 and 5, of which 4 is held.
		entry->store(file, image_range(4, 2));
		const auto missing =
			entry->read(array_ref(destination), image_range(4, 2));

		CHECK( get_file_indices(missing) == index_list({4}) );
		CHECK( get_values<float>(destination) ==
			std::vector<float>(2 * image_size, untouched) );
	}

	SECTION( "nor when the first of them is not" )
	{
		entry->store(file, image_range(1, 2));
		const auto missing =
			entry->read(array_ref(destination), image_range(1, 2));

		CHECK( get_file_indices(missing) == index_list({1}) );
		CHECK( get_values<float>(destination) ==
			std::vector<float>(2 * image_size, untouched) );
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry loads every run a stored region spans",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_whole.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3, 4, 5, 6, 7}, 3);

	// Eight slots in runs of three: three reads.
	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.TIMES(3)
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
	auto destination = make_batch(8);

	entry->store(file, image_range(0, 8));
	const auto missing =
		entry->read(array_ref(destination), image_range(0, 8));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) ==
		count_from(0, 8 * image_size) );
}

TEST_CASE(
	"a buffer_image_scratch_entry reads a patch of a held image",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_patch.raw");
	mock_image_reader file;
	const auto entry = make_entry({5}, 1);

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

	// The second row of image 5, into an array of one row.
	image_transfer_plan patch(image_transfer_shape({1, 2}, 3, 2));
	const std::size_t file_offset[3] = {5, 1, 0};
	const std::size_t array_offset[2] = {0, 0};
	patch.add(make_span(file_offset, 3), make_span(array_offset, 2));
	auto destination =
		make_host_array<float>({1, 2}, numerical_type::float32, untouched);

	entry->store(file, patch);
	const auto missing = entry->read(array_ref(destination), patch);

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == std::vector<float>({22, 23}) );
}

TEST_CASE(
	"a buffer_image_scratch_entry converts to the type of the destination",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_convert.raw");
	mock_image_reader file;
	const auto entry = make_entry({5}, 1);

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
	auto destination =
		make_host_array<double>({1, 2, 2}, numerical_type::float64, -1.0);

	entry->store(file, images({5}));
	const auto missing = entry->read(array_ref(destination), images({5}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<double>(destination) ==
		std::vector<double>({20, 21, 22, 23}) );
}

TEST_CASE(
	"a buffer_image_scratch_entry passes over a plan of another rank",
	"[buffer_image_scratch_entry]"
)
{
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3, 4, 5, 6, 7}, 8);

	// A plan for a file of two axes, where the stack has three.
	image_transfer_plan plan(image_transfer_shape(image_extents, 2, 3));
	const std::size_t file_offset[2] = {0, 0};
	const std::size_t array_offset[3] = {0, 0, 0};
	plan.add(make_span(file_offset, 2), make_span(array_offset, 3));
	auto destination = make_batch(1);

	SECTION( "storing it reads nothing" )
	{
		FORBID_CALL(file, read(trompeloeil::_, trompeloeil::_));

		entry->store(file, plan);
	}

	SECTION( "reading it answers every region" )
	{
		const auto missing = entry->read(array_ref(destination), plan);

		CHECK( missing.get_region_count() == 1 );
		CHECK( get_values<float>(destination) ==
			std::vector<float>(image_size, untouched) );
	}
}

TEST_CASE(
	"a buffer_image_scratch_entry leaves a run absent when loading it fails",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_retry.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3}, 4);
	auto destination = make_batch(1);
	trompeloeil::sequence order;

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.IN_SEQUENCE(order)
		.THROW( std::runtime_error("from the file") );
	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.IN_SEQUENCE(order)
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

	REQUIRE_THROWS_AS( entry->store(file, images({2})), std::runtime_error );
	CHECK( entry->read(array_ref(destination), images({2}))
		.get_region_count() == 1 );

	// The next store reads the file again.
	entry->store(file, images({2}));
	const auto missing = entry->read(array_ref(destination), images({2}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({2}) );
}

TEST_CASE(
	"a buffer_image_scratch_entry loads a run once for two threads",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_threads.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3}, 4);

	REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
		.TIMES(1)
		.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );

	std::thread first([&] { entry->store(file, images({1})); });
	std::thread second([&] { entry->store(file, images({3})); });
	first.join();
	second.join();

	auto destination = make_batch(2);
	const auto missing = entry->read(array_ref(destination), images({1, 3}));

	CHECK( missing.get_region_count() == 0 );
	CHECK( get_values<float>(destination) == values_of_images({1, 3}) );
}

TEST_CASE(
	"a buffer_image_scratch_entry checks the destination of a read",
	"[buffer_image_scratch_entry]"
)
{
	const stack_file stack("buffer_scratch_entry_destination.raw");
	mock_image_reader file;
	const auto entry = make_entry({0, 1, 2, 3}, 4);

	SECTION( "a destination that is not initialized is refused" )
	{
		array destination;

		REQUIRE_THROWS_AS(
			entry->read(array_ref(destination), images({1})),
			std::invalid_argument
		);
	}

	SECTION( "a held image that does not fit the destination is refused" )
	{
		REQUIRE_CALL(file, read(trompeloeil::_, trompeloeil::_))
			.LR_SIDE_EFFECT( stack.get_reader().read(_1, _2) );
		auto destination = make_batch(1);

		entry->store(file, images({0, 1}));

		// The second image lands in a slot the batch does not have.
		REQUIRE_THROWS_AS(
			entry->read(array_ref(destination), images({0, 1})),
			std::out_of_range
		);
	}
}
