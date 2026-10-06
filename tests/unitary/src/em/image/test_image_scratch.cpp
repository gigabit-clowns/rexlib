// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_scratch.hpp>

#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"
#include "mock/mock_image_scratch.hpp"
#include "mock/mock_image_scratch_entry.hpp"

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <trompeloeil.hpp>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// Stacks of eight images of 4 by 4.
const std::vector<std::size_t> stack_extents = {8, 4, 4};
const std::vector<std::size_t> image_extents = {4, 4};

const image_descriptor stack_descriptor(
	make_span(stack_extents),
	2,
	numerical_type::float32
);

using index_list = std::vector<std::size_t>;

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

// Whether a plan is one region that spans a whole stack.
bool spans_whole_stack(const image_transfer_plan &plan)
{
	const std::vector<std::size_t> origin(stack_extents.size(), 0);

	return
		plan.get_region_count() == 1 &&
		to_vector(plan.get_shape().get_extents()) == stack_extents &&
		to_vector(plan.get_file_offset(0)) == origin;
}

// Whether a plan is one whole image for each of some indices of a stack.
bool spans_images(const image_transfer_plan &plan, const index_list &indices)
{
	if (to_vector(plan.get_shape().get_extents()) != image_extents)
	{
		return false;
	}

	index_list planned;
	for (std::size_t region = 0; region < plan.get_region_count(); ++region)
	{
		const auto offset = to_vector(plan.get_file_offset(region));
		if (offset[1] != 0 || offset[2] != 0)
		{
			return false;
		}

		planned.push_back(offset.front());
	}

	return planned == indices;
}

std::shared_ptr<completion> prefetch(
	image_scratch &scratch,
	std::shared_ptr<image_reader_provider> files,
	const std::vector<image_location> &locations
)
{
	synchronous_executor executor;

	return prefetch_scratch_async(
		scratch,
		std::move(files),
		executor,
		make_span(locations)
	);
}

} // anonymous namespace

TEST_CASE(
	"prefetch_scratch_async needs a reader provider",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;

	REQUIRE_THROWS_AS(
		prefetch(scratch, nullptr, {image_location("stack.mrcs", 0)}),
		std::invalid_argument
	);
}

TEST_CASE(
	"prefetch_scratch_async stores what the locations name of each file",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;
	const auto first_entry = std::make_shared<mock_image_scratch_entry>();
	const auto second_entry = std::make_shared<mock_image_scratch_entry>();
	const auto files = std::make_shared<mock_image_reader_provider>();
	const auto first_file = std::make_shared<mock_image_reader>();
	const auto second_file = std::make_shared<mock_image_reader>();
	trompeloeil::sequence order;

	REQUIRE_CALL(scratch, find("stack_0.mrcs")).RETURN(first_entry);
	REQUIRE_CALL(scratch, find("stack_1.mrcs")).RETURN(second_entry);
	ALLOW_CALL(*first_file, get_descriptor())
		.RETURN(std::ref(stack_descriptor));
	ALLOW_CALL(*second_file, get_descriptor())
		.RETURN(std::ref(stack_descriptor));

	// The files are taken in the order the locations first name them, each
	// opened once and stored into its own entry.
	REQUIRE_CALL(*files, acquire("stack_0.mrcs"))
		.IN_SEQUENCE(order)
		.RETURN(first_file);
	REQUIRE_CALL(*first_entry, store(trompeloeil::_, trompeloeil::_))
		.LR_WITH( &_1 == first_file.get() && spans_images(_2, {1, 4}) )
		.IN_SEQUENCE(order);
	REQUIRE_CALL(*files, acquire("stack_1.mrcs"))
		.IN_SEQUENCE(order)
		.RETURN(second_file);
	REQUIRE_CALL(*second_entry, store(trompeloeil::_, trompeloeil::_))
		.LR_WITH( &_1 == second_file.get() && spans_images(_2, {2}) )
		.IN_SEQUENCE(order);

	const auto completion = prefetch(
		scratch,
		files,
		{
			image_location("stack_0.mrcs", 4),
			image_location("stack_1.mrcs", 2),
			image_location("stack_0.mrcs", 1)
		}
	);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"prefetch_scratch_async stores the whole of a file named as a whole",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;
	const auto entry = std::make_shared<mock_image_scratch_entry>();
	const auto files = std::make_shared<mock_image_reader_provider>();
	const auto file = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(scratch, find("stack.mrcs")).RETURN(entry);
	ALLOW_CALL(*file, get_descriptor()).RETURN(std::ref(stack_descriptor));
	REQUIRE_CALL(*files, acquire("stack.mrcs")).RETURN(file);
	REQUIRE_CALL(*entry, store(trompeloeil::_, trompeloeil::_))
		.LR_WITH( &_1 == file.get() && spans_whole_stack(_2) );

	const auto completion = prefetch(
		scratch,
		files,
		{image_location("stack.mrcs", 3), image_location("stack.mrcs")}
	);

	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"prefetch_scratch_async skips a file the scratch has no entry for",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;
	const auto entry = std::make_shared<mock_image_scratch_entry>();
	const auto files = std::make_shared<mock_image_reader_provider>();
	const auto file = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(scratch, find("unheld.mrcs")).RETURN(nullptr);
	REQUIRE_CALL(scratch, find("stack.mrcs")).RETURN(entry);
	ALLOW_CALL(*file, get_descriptor()).RETURN(std::ref(stack_descriptor));

	// The file with no entry is not even opened.
	FORBID_CALL(*files, acquire("unheld.mrcs"));
	REQUIRE_CALL(*files, acquire("stack.mrcs")).RETURN(file);
	REQUIRE_CALL(*entry, store(trompeloeil::_, trompeloeil::_));

	const auto completion = prefetch(
		scratch,
		files,
		{image_location("unheld.mrcs", 0), image_location("stack.mrcs", 0)}
	);

	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"prefetch_scratch_async reports the first failure once every file is "
	"done",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;
	const auto first_entry = std::make_shared<mock_image_scratch_entry>();
	const auto second_entry = std::make_shared<mock_image_scratch_entry>();
	const auto files = std::make_shared<mock_image_reader_provider>();
	const auto second_file = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(scratch, find("absent.mrcs")).RETURN(first_entry);
	REQUIRE_CALL(scratch, find("stack.mrcs")).RETURN(second_entry);
	ALLOW_CALL(*second_file, get_descriptor())
		.RETURN(std::ref(stack_descriptor));

	REQUIRE_CALL(*files, acquire("absent.mrcs"))
		.SIDE_EFFECT( throw std::out_of_range("from the provider") )
		.RETURN(nullptr);
	FORBID_CALL(*first_entry, store(trompeloeil::_, trompeloeil::_));

	// The file after the one that failed is still loaded.
	REQUIRE_CALL(*files, acquire("stack.mrcs")).RETURN(second_file);
	REQUIRE_CALL(*second_entry, store(trompeloeil::_, trompeloeil::_));

	const auto completion = prefetch(
		scratch,
		files,
		{image_location("absent.mrcs", 0), image_location("stack.mrcs", 0)}
	);

	CHECK( completion->is_ready() );
	CHECK_THROWS_AS( completion->get(), std::out_of_range );
}

TEST_CASE(
	"prefetch_scratch_async of no location is done at once",
	"[image_scratch]"
)
{
	mock_image_scratch scratch;
	const auto files = std::make_shared<mock_image_reader_provider>();

	const auto completion = prefetch(scratch, files, {});

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}
