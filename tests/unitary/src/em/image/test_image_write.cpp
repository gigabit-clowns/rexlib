// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_sink.hpp>

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_writer.hpp"
#include "mock/mock_image_writer_provider.hpp"

#include <cstddef>
#include <memory>
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

std::shared_ptr<image_sink> make_sink(
	std::shared_ptr<image_writer_provider> writers
)
{
	return std::make_shared<image_sink>(
		std::move(writers),
		std::make_shared<synchronous_executor>()
	);
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
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto sink = make_sink(writers);
	// No expectations set on `writers`: none of these calls may reach it.

	SECTION( "a source with no extents" )
	{
		const std::vector<image_location> locations;

		REQUIRE_THROWS_AS(
			write_batch_async(
			*sink,
			make_const_array({}), make_span(locations)),
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
			*sink,
			make_const_array({3, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"write_batch_async refuses to mix indexed and unindexed locations",
	"[image_write]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto sink = make_sink(writers);
	// No expectations set on `writers`: a rejected batch must not reach it.

	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			write_batch_async(
			*sink,
			make_const_array({2, 4, 4}), make_span(locations)),
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
			*sink,
			make_const_array({2, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"write_batch_async resolves an empty batch without touching the sink",
	"[image_write]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto sink = make_sink(writers);
	// No expectations set on `writers`: acquiring anything would violate.

	const std::vector<image_location> locations;
	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({0, 4, 4}), make_span(locations));

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"write_batch_async addresses the whole file for unindexed locations",
	"[image_write]"
)
{
	// No position_in_stack: every location names a file written as a whole
	// image, so the file rank matches the core rank and every file offset
	// stays at the origin.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer_a = std::make_shared<mock_image_writer>();
	const auto writer_b = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("a.mrc")).RETURN(writer_a);
	REQUIRE_CALL(*writers, acquire("b.mrc")).RETURN(writer_b);

	REQUIRE_CALL(*writer_a, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);
	REQUIRE_CALL(*writer_b, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto sink = make_sink(writers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"write_batch_async writes a stack a batch at a time",
	"[image_write]"
)
{
	// The case this exists for: one file acquired once, and the slots of the
	// batch written as its regions, each the shape of one element.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("particles.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{6, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{8, 0, 0} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto sink = make_sink(writers);
	const std::vector<image_location> locations = {
		image_location("particles.mrcs", 6),
		image_location("particles.mrcs", 7),
		image_location("particles.mrcs", 8)
	};

	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"write_batch_async gives every slot of a batch its own region",
	"[image_write]"
)
{
	// Two pairs of consecutive positions with a gap between them, which a
	// plan describes the same way as any other four slots.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("particles.mrcs")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 4 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{4, 0, 0} &&
			to_vector(_2.get_file_offset(3)) ==
				std::vector<std::size_t>{5, 0, 0}
		);

	const auto sink = make_sink(writers);
	const std::vector<image_location> locations = {
		image_location("particles.mrcs", 0),
		image_location("particles.mrcs", 1),
		image_location("particles.mrcs", 4),
		image_location("particles.mrcs", 5)
	};

	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({4, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"write_batch_async spreads one batch over the stacks it names",
	"[image_write]"
)
{
	// Nothing binds a batch to one stack: each file is acquired once and
	// gets the slots that named it.
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto first = std::make_shared<mock_image_writer>();
	const auto second = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("first.mrcs")).RETURN(first);
	REQUIRE_CALL(*writers, acquire("second.mrcs")).RETURN(second);

	REQUIRE_CALL(*first, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{2, 0, 0}
		);
	REQUIRE_CALL(*second, write(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto sink = make_sink(writers);
	const std::vector<image_location> locations = {
		image_location("first.mrcs", 0),
		image_location("second.mrcs", 0),
		image_location("first.mrcs", 1)
	};

	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"write_batch_async's completion reports what the sink threw",
	"[image_write]"
)
{
	const auto writers = std::make_shared<mock_image_writer_provider>();
	const auto writer = std::make_shared<mock_image_writer>();

	REQUIRE_CALL(*writers, acquire("a.mrc")).RETURN(writer);
	REQUIRE_CALL(*writer, write(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::out_of_range("past the end of the stack") );

	const auto sink = make_sink(writers);
	const std::vector<image_location> locations = { image_location("a.mrc") };

	const auto completion =
		write_batch_async(
			*sink,
			make_const_array({1, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::out_of_range );
}
