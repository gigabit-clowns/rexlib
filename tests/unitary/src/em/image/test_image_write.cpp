// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/image_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "fixtures/format_manager_fixture.hpp"
#include "mock/mock_image_sink.hpp"
#include "mock/mock_image_writer.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

array make_array(
	const std::vector<std::size_t> &extents,
	numerical_type data_type
)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor =
		make_contiguous_array_descriptor(make_span(extents), data_type);
	return array(storage, descriptor);
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

std::vector<std::size_t> extents_of(const const_array &arr)
{
	std::vector<std::size_t> result;
	arr.get_descriptor().get_layout().get_extents(result);
	return result;
}

const_array make_const_array(const std::vector<std::size_t> &extents)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_contiguous_array_descriptor(
		make_span(extents),
		numerical_type::float32
	);
	array source(storage, descriptor);
	return source.share_const();
}

} // anonymous namespace

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_single(...) creates one image or volume of the array's extents",
	"[image_write]"
)
{
	const std::vector<std::size_t> extents = {2, 3, 4};
	const auto arr = make_array(extents, numerical_type::float32);
	const image_descriptor expected(
		make_span(extents),
		extents.size(),
		numerical_type::float32
	);

	auto &format = add_format(backend_priority::normal);
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _1.get_path() == "out.mrc" && _2 == expected )
		.RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == extents &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);
	ALLOW_CALL(*writer, flush());

	write_single(arr, "out.mrc", *get_manager());
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_single(...) converts to the data type it is given",
	"[image_write]"
)
{
	const std::vector<std::size_t> extents = {2, 3};
	const auto arr = make_array(extents, numerical_type::float32);
	const image_descriptor expected(
		make_span(extents),
		extents.size(),
		numerical_type::int16
	);

	auto &format = add_format(backend_priority::normal);
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2 == expected )
		.RETURN(writer);
	ALLOW_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	ALLOW_CALL(*writer, flush());

	write_single(arr, "out.mrc", *get_manager(), numerical_type::int16);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_single(...) refuses an array with no extents",
	"[image_write]"
)
{
	// No expectation on open: creating the file would violate.
	add_format(backend_priority::normal);

	REQUIRE_THROWS_AS(
		write_single(
			make_array({}, numerical_type::float32),
			"out.mrc",
			*get_manager()
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_stack(...) stacks the file along the leading extent",
	"[image_write]"
)
{
	// Extents that could be one volume, written as a stack of two images.
	const std::vector<std::size_t> extents = {2, 3, 4};
	const auto arr = make_array(extents, numerical_type::float32);

	auto &format = add_format(backend_priority::normal);
	const auto writer = std::make_shared<mock_image_writer>();
	ALLOW_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	ALLOW_CALL(*writer, flush());

	SECTION( "in the data type of the array" )
	{
		const image_descriptor expected(
			make_span(extents),
			2,
			numerical_type::float32
		);

		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.LR_WITH( _1.get_path() == "stack.mrcs" && _2 == expected )
			.RETURN(writer);

		write_stack(arr, "stack.mrcs", *get_manager());
	}

	SECTION( "in the data type it is given" )
	{
		const image_descriptor expected(
			make_span(extents),
			2,
			numerical_type::int16
		);

		REQUIRE_CALL(
			format,
			open(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.LR_WITH( _2 == expected )
			.RETURN(writer);

		write_stack(arr, "stack.mrcs", *get_manager(), numerical_type::int16);
	}
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_stack(...) refuses an array with nothing to stack",
	"[image_write]"
)
{
	// No expectation on open: creating the file would violate.
	add_format(backend_priority::normal);

	REQUIRE_THROWS_AS(
		write_stack(
			make_array({4}, numerical_type::float32),
			"stack.mrcs",
			*get_manager()
		),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write(..., descriptor) creates the file the descriptor states",
	"[image_write]"
)
{
	// Extents that could be one volume, stated as a stack of two images, and
	// stored as a narrower type than the array carries.
	const std::vector<std::size_t> extents = {2, 3, 4};
	const auto arr = make_array(extents, numerical_type::float32);
	const image_descriptor descriptor(
		make_span(extents),
		2,
		numerical_type::int16
	);

	auto &format = add_format(backend_priority::normal);
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.LR_WITH( _1.get_path() == "stack.mrcs" && _2 == descriptor )
		.RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == extents
		);
	ALLOW_CALL(*writer, flush());

	write(arr, "stack.mrcs", *get_manager(), descriptor);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write(..., descriptor) refuses a descriptor of other extents",
	"[image_write]"
)
{
	const std::vector<std::size_t> file_extents = {3, 2};
	const auto arr = make_array({2, 3}, numerical_type::float32);
	const image_descriptor descriptor(
		make_span(file_extents),
		2,
		numerical_type::float32
	);

	// No expectation on open: creating the file would violate.
	add_format(backend_priority::normal);

	REQUIRE_THROWS_AS(
		write(arr, "out.mrc", *get_manager(), descriptor),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	write_format_manager_fixture,
	"write_single(...) flushes after writing",
	"[image_write]"
)
{
	const auto arr = make_array({2, 3}, numerical_type::float32);

	auto &format = add_format(backend_priority::normal);
	const auto writer = std::make_shared<mock_image_writer>();
	trompeloeil::sequence order;

	REQUIRE_CALL(format, open(trompeloeil::_, trompeloeil::_, trompeloeil::_))
		.RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.IN_SEQUENCE(order);
	REQUIRE_CALL(*writer, flush())
		.IN_SEQUENCE(order);

	write_single(arr, "out.mrc", *get_manager());
}

TEST_CASE(
	"write_batch_async validates the source array",
	"[image_write]"
)
{
	// No expectations set on `sink`: none of these calls may reach it.
	mock_image_sink sink;

	SECTION( "a source with no extents" )
	{
		const std::vector<image_location> locations;

		REQUIRE_THROWS_AS(
			write_batch_async(
				sink,
				make_const_array({}),
				make_span(locations)
			),
			std::invalid_argument
		);
	}

	SECTION( "a batch size that does not match the location count" )
	{
		const std::vector<image_location> locations = {
			image_location("a.mrc"),
			image_location("b.mrc")
		};

		REQUIRE_THROWS_AS(
			write_batch_async(
				sink,
				make_const_array({3, 4, 4}),
				make_span(locations)
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"write_batch_async refuses to mix indexed and unindexed locations",
	"[image_write]"
)
{
	// No expectations set on `sink`: a rejected batch must not reach it.
	mock_image_sink sink;

	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			write_batch_async(
				sink,
				make_const_array({2, 4, 4}),
				make_span(locations)
			),
			std::invalid_argument
		);
	}

	SECTION( "an indexed location following an unindexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("plain.mrc"),
			image_location("stack.mrcs", 0)
		};

		REQUIRE_THROWS_AS(
			write_batch_async(
				sink,
				make_const_array({2, 4, 4}),
				make_span(locations)
			),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"write_batch_async hands an empty batch over as an empty plan",
	"[image_write]"
)
{
	mock_image_sink sink;
	const auto done = std::make_shared<counting_completion>(0);

	REQUIRE_CALL(sink, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH( _2.get_region_count() == 0 )
		.RETURN(done);

	const std::vector<image_location> locations;
	const auto completion = write_batch_async(
		sink,
		make_const_array({0, 4, 4}),
		make_span(locations)
	);

	CHECK( completion == done );
}

TEST_CASE(
	"write_batch_async addresses the whole file for unindexed locations",
	"[image_write]"
)
{
	// No index in a stack: every location names a file written as a whole
	// image, so the file rank is the core rank and every file offset stays
	// at the origin.
	mock_image_sink sink;

	REQUIRE_CALL(sink, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			extents_of(_1) == std::vector<std::size_t>{2, 3, 5} &&
			_2.get_region_count() == 2 &&
			_2.get_file_rank() == 2 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == std::vector<std::size_t>{3, 5} &&
			_2.get_file(_2.get_region_file(0)) == "a.mrc" &&
			_2.get_file(_2.get_region_file(1)) == "b.mrc" &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0}
		)
		.RETURN(std::make_shared<counting_completion>(0));

	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	write_batch_async(
		sink,
		make_const_array({2, 3, 5}),
		make_span(locations)
	);
}

TEST_CASE(
	"write_batch_async writes a stack a batch at a time",
	"[image_write]"
)
{
	// Every location names the same stack, so the plan names one file, and
	// each slot of the batch becomes one region of it, placed at the slot's
	// index in the stack.
	mock_image_sink sink;

	REQUIRE_CALL(sink, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_file_count() == 1 &&
			_2.get_file(0) == "particles.mrcs" &&
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{6, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{7, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{8, 0, 0} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		)
		.RETURN(std::make_shared<counting_completion>(0));

	const std::vector<image_location> locations = {
		image_location("particles.mrcs", 6),
		image_location("particles.mrcs", 7),
		image_location("particles.mrcs", 8)
	};

	write_batch_async(
		sink,
		make_const_array({3, 4, 4}),
		make_span(locations)
	);
}

TEST_CASE(
	"write_batch_async returns the completion of the sink",
	"[image_write]"
)
{
	mock_image_sink sink;
	const auto pending = std::make_shared<counting_completion>(1);

	REQUIRE_CALL(sink, write(trompeloeil::_, trompeloeil::_))
		.RETURN(pending);

	const std::vector<image_location> locations = { image_location("a.mrc") };
	const auto completion = write_batch_async(
		sink,
		make_const_array({1, 3, 5}),
		make_span(locations)
	);

	CHECK( completion == pending );
}
