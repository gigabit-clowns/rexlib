// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/counting_completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_sink.hpp"
#include "mock/mock_image_writer.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// What one call to open was handed, so a test can check what image_write.cpp
// asked for rather than reach into a mock's own bookkeeping.
struct open_record
{
	std::vector<std::size_t> extents;
	std::size_t core_rank = 0;
	numerical_type data_type = numerical_type::unknown;
};

// A format that always claims a file, records what it was asked to create
// and hands back one fixed writer.
class staged_write_format final
	: public image_write_format
{
public:
	staged_write_format(
		std::shared_ptr<image_writer> writer,
		std::shared_ptr<open_record> record
	)
		: m_writer(std::move(writer))
		, m_record(std::move(record))
	{
	}

	std::string get_name() const override
	{
		return "staged";
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return backend_priority::normal;
	}

	std::shared_ptr<image_writer> open(
		const image_probe &,
		span<const std::size_t> extents,
		std::size_t core_rank,
		numerical_type data_type,
		const image_metadata &
	) const override
	{
		m_record->extents.assign(extents.begin(), extents.end());
		m_record->core_rank = core_rank;
		m_record->data_type = data_type;
		return m_writer;
	}

private:
	std::shared_ptr<image_writer> m_writer;
	std::shared_ptr<open_record> m_record;
};

void register_writer(
	image_write_format_manager &manager,
	std::shared_ptr<image_writer> writer,
	std::shared_ptr<open_record> record
)
{
	manager.register_format(
		std::make_unique<staged_write_format>(
			std::move(writer),
			std::move(record)
		)
	);
}

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

TEST_CASE(
	"write(...) creates the file over the array's own shape and data type",
	"[image_write]"
)
{
	const std::vector<std::size_t> extents = {2, 3, 4};
	const auto arr = make_array(extents, numerical_type::float32);

	const auto writer = std::make_shared<mock_image_writer>();
	const auto record = std::make_shared<open_record>();

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
	REQUIRE_CALL(*writer, flush());

	image_write_format_manager manager;
	register_writer(manager, writer, record);
	write(arr, "out.mrc", manager);

	CHECK( record->extents == extents );
	CHECK( record->core_rank == extents.size() );
	CHECK( record->data_type == numerical_type::float32 );
}

TEST_CASE(
	"write(...) honors an explicit data_type over the array's own",
	"[image_write]"
)
{
	const auto arr = make_array({2, 3}, numerical_type::float32);

	const auto writer = std::make_shared<mock_image_writer>();
	const auto record = std::make_shared<open_record>();

	ALLOW_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	ALLOW_CALL(*writer, flush());

	image_write_format_manager manager;
	register_writer(manager, writer, record);
	write(
		arr,
		"out.mrc",
		manager,
		numerical_type::int16
	);

	CHECK( record->data_type == numerical_type::int16 );
}

TEST_CASE(
	"write(...) flushes after writing",
	"[image_write]"
)
{
	const auto arr = make_array({2, 3}, numerical_type::float32);

	const auto writer = std::make_shared<mock_image_writer>();
	const auto record = std::make_shared<open_record>();

	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_));
	REQUIRE_CALL(*writer, flush());

	image_write_format_manager manager;
	register_writer(manager, writer, record);
	write(arr, "out.mrc", manager);
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
	// No stack index: every location names a file written as a whole image,
	// so the file rank is the core rank and every file offset stays at the
	// origin.
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
	// stack index.
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
