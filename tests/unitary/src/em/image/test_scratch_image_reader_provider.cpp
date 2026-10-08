// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/scratch_image_reader_provider.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"
#include "mock/mock_image_scratch.hpp"
#include "mock/mock_image_scratch_entry.hpp"

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_transfer_plan.hpp>
#include <rexlib/em/image/image_transfer_shape.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <trompeloeil.hpp>
#include <utility>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> image_extents = {4, 4};
const std::vector<std::size_t> batch_extents = {1, 4, 4};

array make_test_array()
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto layout = strided_layout::make_contiguous_layout(
		make_span(batch_extents)
	);
	array_descriptor descriptor(layout, numerical_type::float32);
	return array(storage, std::move(descriptor));
}

// Image `index` of a stack, landing in the only slot of a batch of one.
image_transfer_plan make_plan(std::size_t index)
{
	image_transfer_plan plan(image_transfer_shape(image_extents, 3, 3));
	const std::array<std::size_t, 3> file_offset = {index, 0, 0};
	const std::array<std::size_t, 3> array_offset = {0, 0, 0};
	plan.add(make_span(file_offset), make_span(array_offset));

	return plan;
}

} // anonymous namespace

TEST_CASE(
	"a scratch_image_reader_provider needs a backing provider and a scratch",
	"[scratch_image_reader_provider]"
)
{
	SECTION( "a null backing provider is refused" )
	{
		REQUIRE_THROWS_AS(
			scratch_image_reader_provider(
				nullptr,
				std::make_shared<mock_image_scratch>()
			),
			std::invalid_argument
		);
	}

	SECTION( "a null scratch is refused" )
	{
		REQUIRE_THROWS_AS(
			scratch_image_reader_provider(
				std::make_shared<mock_image_reader_provider>(),
				nullptr
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"a scratch_image_reader_provider serves a file the scratch holds "
	"nothing of as its backing provider gave it",
	"[scratch_image_reader_provider]"
)
{
	const auto backing = std::make_shared<mock_image_reader_provider>();
	const auto scratch = std::make_shared<mock_image_scratch>();
	const auto file = std::make_shared<mock_image_reader>();
	scratch_image_reader_provider provider(backing, scratch);

	REQUIRE_CALL(*backing, acquire("micrograph.mrc")).RETURN(file);
	REQUIRE_CALL(*scratch, find("micrograph.mrc")).RETURN(nullptr);

	CHECK( provider.acquire("micrograph.mrc") == file );
}

TEST_CASE(
	"a scratch_image_reader_provider serves a file the scratch holds an "
	"entry of through that entry",
	"[scratch_image_reader_provider]"
)
{
	const auto backing = std::make_shared<mock_image_reader_provider>();
	const auto scratch = std::make_shared<mock_image_scratch>();
	const auto entry = std::make_shared<mock_image_scratch_entry>();
	const auto file = std::make_shared<mock_image_reader>();
	scratch_image_reader_provider provider(backing, scratch);

	REQUIRE_CALL(*backing, acquire("stack.mrcs")).RETURN(file);
	REQUIRE_CALL(*scratch, find("stack.mrcs")).RETURN(entry);

	const auto reader = provider.acquire("stack.mrcs");

	REQUIRE( reader != nullptr );
	REQUIRE( reader != file );

	SECTION( "the reader reports what the backing reader reports" )
	{
		const std::vector<std::size_t> extents = {8, 4, 4};
		const image_descriptor descriptor(
			make_span(extents),
			2,
			numerical_type::float32
		);
		ALLOW_CALL(*file, get_descriptor()).LR_RETURN(std::ref(descriptor));

		CHECK( &reader->get_descriptor() == &descriptor );
	}

	SECTION( "what the entry holds is not read from the backing reader" )
	{
		const auto regions = make_plan(5);
		const auto nothing =
			image_transfer_plan(image_transfer_shape(image_extents, 3, 3));
		auto destination = make_test_array();

		REQUIRE_CALL(*entry, read(trompeloeil::_, trompeloeil::_))
			.LR_RETURN(nothing);
		FORBID_CALL(*file, read(trompeloeil::_, trompeloeil::_));

		reader->read(array_ref(destination), regions);
	}

	SECTION( "what it does not hold is taken in out of the backing reader" )
	{
		const auto regions = make_plan(5);
		const auto nothing =
			image_transfer_plan(image_transfer_shape(image_extents, 3, 3));
		auto destination = make_test_array();
		trompeloeil::sequence order;

		REQUIRE_CALL(*entry, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_RETURN(regions);
		REQUIRE_CALL(*entry, store(trompeloeil::_, trompeloeil::_))
			.LR_WITH( &_1 == file.get() )
			.IN_SEQUENCE(order);
		REQUIRE_CALL(*entry, read(trompeloeil::_, trompeloeil::_))
			.IN_SEQUENCE(order)
			.LR_RETURN(nothing);

		reader->read(array_ref(destination), regions);
	}
}

TEST_CASE(
	"a scratch_image_reader_provider reports what its backing provider "
	"reported",
	"[scratch_image_reader_provider]"
)
{
	const auto backing = std::make_shared<mock_image_reader_provider>();
	const auto scratch = std::make_shared<mock_image_scratch>();
	scratch_image_reader_provider provider(backing, scratch);

	REQUIRE_CALL(*backing, acquire("absent.mrcs"))
		.SIDE_EFFECT( throw std::runtime_error("from the backing") )
		.RETURN(nullptr);
	ALLOW_CALL(*scratch, find("absent.mrcs"))
		.RETURN(std::make_shared<mock_image_scratch_entry>());

	REQUIRE_THROWS_AS( provider.acquire("absent.mrcs"), std::runtime_error );
}
