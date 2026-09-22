// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_read.hpp>

#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/hardware/device_context.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/device_session.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_location.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_source.hpp>
#include <rexlib/em/image/index_table.hpp>
#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/concurrency/synchronous_executor.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "../../core/hardware/mock/mock_command_queue.hpp"
#include "../../core/hardware/mock/mock_device.hpp"
#include "../../core/hardware/mock/mock_memory_allocator.hpp"
#include "../../core/hardware/mock/mock_memory_resource.hpp"
#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_reader_provider.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <trompeloeil.hpp>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// An execution_context whose allocation path is entirely mocked, so
// rexlib::empty can hand out as many arrays as a test needs without any
// real device behind it. Nothing reads the bytes of an allocated array back,
// so a fresh mock_buffer with no expectations of its own is enough.
class mocked_execution_context_fixture
{
public:
	mocked_execution_context_fixture()
		: device(std::make_shared<mock_device>())
		, host_allocator(std::make_shared<mock_memory_allocator>())
		, device_allocator(std::make_shared<mock_memory_allocator>())
		, queue(std::make_shared<mock_command_queue>())
	{
		device_properties properties;
		properties.set_optimal_data_alignment(128);

		// Named and kept alive for the fixture's own lifetime, rather than
		// left as bare statements: get_max_alignment and allocate are only
		// called later, from inside a TEST_CASE_METHOD body, well after this
		// constructor - and, unlike this constructor's - has returned.
		expectations.emplace_back(NAMED_REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::host)
		)
			.LR_RETURN(host_resource));
		expectations.emplace_back(NAMED_REQUIRE_CALL(
			*device,
			get_memory_resource(memory_resource_affinity::device)
		)
			.LR_RETURN(device_resource));
		expectations.emplace_back(NAMED_REQUIRE_CALL(
			host_resource, create_allocator()
		)
			.RETURN(host_allocator));
		expectations.emplace_back(NAMED_REQUIRE_CALL(
			device_resource, create_allocator()
		)
			.RETURN(device_allocator));
		expectations.emplace_back(NAMED_REQUIRE_CALL(
			*device, create_command_queue()
		)
			.RETURN(queue));

		expectations.emplace_back(NAMED_ALLOW_CALL(
			*host_allocator, get_max_alignment()
		)
			.RETURN(128UL));
		expectations.emplace_back(NAMED_ALLOW_CALL(
			*host_allocator,
			allocate(trompeloeil::_, trompeloeil::_, trompeloeil::_)
		)
			.RETURN(std::make_shared<mock_buffer>()));

		const auto session = std::make_shared<device_session>(
			device,
			std::move(properties)
		);
		context = execution_context(device_context(session), nullptr);
	}

	std::shared_ptr<mock_device> device;
	std::shared_ptr<mock_memory_allocator> host_allocator;
	std::shared_ptr<mock_memory_allocator> device_allocator;
	std::shared_ptr<mock_command_queue> queue;
	mock_memory_resource host_resource;
	mock_memory_resource device_resource;
	execution_context context;

private:
	std::vector<std::unique_ptr<trompeloeil::expectation>> expectations;
};

// A format that always claims a file and hands back one fixed reader, and
// remembers the path it was asked about, so a test controls exactly what
// image_read.cpp sees without touching the filesystem or the real registry.
class staged_read_format final
	: public image_read_format
{
public:
	staged_read_format(
		std::shared_ptr<image_reader> reader,
		std::shared_ptr<std::string> opened_path = nullptr
	)
		: m_reader(std::move(reader))
		, m_opened_path(std::move(opened_path))
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

	std::shared_ptr<image_reader> open(const image_probe &probe) const override
	{
		if (m_opened_path)
		{
			*m_opened_path = probe.get_path();
		}
		return m_reader;
	}

private:
	std::shared_ptr<image_reader> m_reader;
	std::shared_ptr<std::string> m_opened_path;
};

void register_reader(
	image_read_format_manager &manager,
	std::shared_ptr<image_reader> reader,
	std::shared_ptr<std::string> opened_path = nullptr
)
{
	manager.register_format(
		std::make_unique<staged_read_format>(
			std::move(reader),
			std::move(opened_path)
		)
	);
}

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

std::vector<std::size_t> extents_of(const array &arr)
{
	std::vector<std::size_t> result;
	arr.get_descriptor().get_layout().get_extents(result);
	return result;
}

// The shapes the asynchronous cases below read from: a plain image, a stack
// of them, and a volume. Named apart because a batch of whole elements and a
// batch of patches want files of very different sizes.
const std::vector<std::size_t> batch_image_extents = {3, 5};
const std::vector<std::size_t> batch_stack_extents = {6, 4, 4};
const std::vector<std::size_t> patch_image_extents = {100, 100};
const std::vector<std::size_t> patch_stack_extents = {6, 100, 100};
const std::vector<std::size_t> volume_extents = {60, 60, 60};

array make_array(const std::vector<std::size_t> &extents)
{
	const auto storage = std::make_shared<mock_buffer>();
	const auto descriptor = make_contiguous_array_descriptor(
		make_span(extents),
		numerical_type::float32
	);
	return array(storage, descriptor);
}

std::shared_ptr<image_source> make_source(
	std::shared_ptr<image_reader_provider> readers
)
{
	return std::make_shared<image_source>(
		std::move(readers),
		std::make_shared<synchronous_executor>()
	);
}

index_table make_positions(
	const std::vector<std::vector<std::size_t>> &positions,
	std::size_t rank
)
{
	index_table result(rank);
	for (const auto &position : positions)
	{
		result.add(make_span(position));
	}

	return result;
}

} // namespace

TEST_CASE_METHOD(
	mocked_execution_context_fixture,
	"read(path, ...) reads a file into an array covering its whole extents",
	"[image_read]"
)
{
	const std::vector<std::size_t> extents = {3, 5};
	const auto reader = std::make_shared<mock_image_reader>();
	const auto opened_path = std::make_shared<std::string>();

	ALLOW_CALL(*reader, get_extents()).LR_RETURN(make_span(extents));
	ALLOW_CALL(*reader, get_data_type()).RETURN(numerical_type::float32);
	ALLOW_CALL(*reader, get_core_rank()).RETURN(std::size_t(2));

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 2 &&
			_2.get_array_rank() == 2 &&
			to_vector(_2.get_extents()) == extents &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0}
		);

	image_read_format_manager manager;
	register_reader(manager, reader, opened_path);
	const auto result = read("plane.mrc", manager, context);

	CHECK( *opened_path == "plane.mrc" );
	CHECK( extents_of(result) == extents );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::float32 );
}

TEST_CASE_METHOD(
	mocked_execution_context_fixture,
	"read(location, ...) reads the whole file when it carries no position",
	"[image_read]"
)
{
	const std::vector<std::size_t> extents = {3, 5};
	const auto reader = std::make_shared<mock_image_reader>();
	const auto opened_path = std::make_shared<std::string>();

	ALLOW_CALL(*reader, get_extents()).LR_RETURN(make_span(extents));
	ALLOW_CALL(*reader, get_data_type()).RETURN(numerical_type::float32);
	ALLOW_CALL(*reader, get_core_rank()).RETURN(std::size_t(2));

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 2 &&
			_2.get_array_rank() == 2 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0}
		);

	image_read_format_manager manager;
	register_reader(manager, reader, opened_path);
	const auto result =
		read(image_location("plane.mrc"), manager, context);

	CHECK( *opened_path == "plane.mrc" );
	CHECK( extents_of(result) == extents );
}

TEST_CASE_METHOD(
	mocked_execution_context_fixture,
	"read(location, ...) reads one stack position into its core shape",
	"[image_read]"
)
{
	// A stack of 4 planes of 3x5: the slowest axis is the stack axis and
	// the trailing two are one plane's core shape.
	const std::vector<std::size_t> file_extents = {4, 3, 5};
	const std::vector<std::size_t> core_extents = {3, 5};
	const auto reader = std::make_shared<mock_image_reader>();

	ALLOW_CALL(*reader, get_extents()).LR_RETURN(make_span(file_extents));
	ALLOW_CALL(*reader, get_data_type()).RETURN(numerical_type::int16);
	ALLOW_CALL(*reader, get_core_rank()).RETURN(std::size_t(2));

	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 2 &&
			to_vector(_2.get_extents()) == core_extents &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{2, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0}
		);

	image_read_format_manager manager;
	register_reader(manager, reader);
	const auto result =
		read(image_location("stack.mrcs", 2), manager, context);

	CHECK( extents_of(result) == core_extents );
	CHECK( result.get_descriptor().get_data_type() == numerical_type::int16 );
}

TEST_CASE(
	"read_batch_async validates the destination array",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto source = make_source(readers);
	// No expectations set on `readers`: none of these calls may reach it.

	SECTION( "a destination with no extents" )
	{
		const std::vector<image_location> locations;

		REQUIRE_THROWS_AS(
			read_batch_async(
			*source,
			make_array({}), make_span(locations)),
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
			read_batch_async(
			*source,
			make_array({3, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"read_batch_async refuses to mix indexed and unindexed locations",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto source = make_source(readers);
	// No expectations set on `readers`: a rejected batch must not reach it.

	SECTION( "an unindexed location following an indexed one" )
	{
		const std::vector<image_location> locations = {
			image_location("stack.mrcs", 0),
			image_location("plain.mrc")
		};

		REQUIRE_THROWS_AS(
			read_batch_async(
			*source,
			make_array({2, 4, 4}), make_span(locations)),
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
			read_batch_async(
			*source,
			make_array({2, 4, 4}), make_span(locations)),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"read_batch_async resolves an empty batch without touching the source",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto source = make_source(readers);
	// No expectations set on `readers`: acquiring anything would violate.

	const std::vector<image_location> locations;
	const auto completion = read_batch_async(
			*source,
			make_array({0, 4, 4}), make_span(locations));

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_batch_async addresses the whole file for unindexed locations",
	"[image_read]"
)
{
	// No position_in_stack: every location names a file read as a whole
	// image, so the file rank matches the core rank and every file offset
	// stays at the origin.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader_a = std::make_shared<mock_image_reader>();
	const auto reader_b = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader_a);
	REQUIRE_CALL(*readers, acquire("b.mrc")).RETURN(reader_b);
	ALLOW_CALL(*reader_a, get_extents()).RETURN(make_span(batch_image_extents));
	ALLOW_CALL(*reader_b, get_extents()).RETURN(make_span(batch_image_extents));

	REQUIRE_CALL(*reader_a, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);
	REQUIRE_CALL(*reader_b, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto source = make_source(readers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("b.mrc")
	};

	const auto completion =
		read_batch_async(
			*source,
			make_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_batch_async groups repeated unindexed locations onto one file",
	"[image_read]"
)
{
	// The same whole file read into two batch slots must be acquired once
	// and delivered as two regions of the same reader call.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(batch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto source = make_source(readers);
	const std::vector<image_location> locations = {
		image_location("a.mrc"),
		image_location("a.mrc")
	};

	const auto completion =
		read_batch_async(
			*source,
			make_array({2, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_batch_async uses the stack position as the file offset",
	"[image_read]"
)
{
	// Every location carries a position_in_stack, so the file rank grows
	// to match the array rank and that position becomes the leading file
	// offset while the array offset's leading axis tracks the batch slot.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(batch_stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) == std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{2, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{5, 0, 0} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto source = make_source(readers);
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 2),
		image_location("stack.mrcs", 0),
		image_location("stack.mrcs", 5)
	};

	const auto completion =
		read_batch_async(
			*source,
			make_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_batch_async reads a stack a batch at a time",
	"[image_read]"
)
{
	// The slots of one batch take consecutive positions of one stack, which
	// is one region each: the whole batch is one hyperrectangle, but saying
	// so needs a set of extents of its own and a plan holds one for every
	// region in it.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(batch_stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{4, 4} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{2, 0, 0} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{4, 0, 0} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto source = make_source(readers);
	const std::vector<image_location> locations = {
		image_location("stack.mrcs", 2),
		image_location("stack.mrcs", 3),
		image_location("stack.mrcs", 4)
	};

	const auto completion =
		read_batch_async(
			*source,
			make_array({3, 4, 4}), make_span(locations));

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_batch_async's completion reports what the source threw",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(batch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	const auto source = make_source(readers);
	const std::vector<image_location> locations = { image_location("a.mrc") };

	const auto completion =
		read_batch_async(
			*source,
			make_array({1, 3, 5}), make_span(locations));

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}

TEST_CASE(
	"read_patches_async validates the destination against the positions",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto source = make_source(readers);
	const image_location location("a.mrc");
	// No expectations set on `readers`: none of these calls may reach it.

	SECTION( "a destination with no extents" )
	{
		const auto positions = make_positions({}, 2);

		REQUIRE_THROWS_AS(
			read_patches_async(
			*source,
			make_array({}), location, positions),
			std::invalid_argument
		);
	}

	SECTION( "a batch size that does not match the position count" )
	{
		const auto positions = make_positions({{10, 10}, {20, 20}}, 2);

		REQUIRE_THROWS_AS(
			read_patches_async(
			*source,
			make_array({3, 10, 10}), location, positions),
			std::invalid_argument
		);
	}

	SECTION( "positions that do not have the rank of one patch" )
	{
		const auto positions = make_positions({{10, 10, 10}}, 3);

		REQUIRE_THROWS_AS(
			read_patches_async(
			*source,
			make_array({1, 10, 10}), location, positions),
			std::invalid_argument
		);
	}
}

TEST_CASE(
	"read_patches_async resolves an empty batch without touching the source",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto source = make_source(readers);
	// No expectations set on `readers`: acquiring anything would violate.

	const auto positions = make_positions({}, 2);
	const auto completion =
		read_patches_async(
			*source,
			make_array({0, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion != nullptr );
	CHECK( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async puts a position at the middle of its patch",
	"[image_read]"
)
{
	// The corner of a patch is its position less half its extent, so a
	// patch of ten centred at fifty starts at forty-five, and one of nine
	// centred there starts at forty-six.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_image_extents));

	const auto source = make_source(readers);
	const auto positions = make_positions({{50, 50}}, 2);

	SECTION( "an even extent" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH(
				to_vector(_2.get_extents()) ==
					std::vector<std::size_t>{10, 10} &&
				to_vector(_2.get_file_offset(0)) ==
					std::vector<std::size_t>{45, 45}
			);

		const auto completion =
			read_patches_async(
			*source,
			make_array({1, 10, 10}), image_location("a.mrc"),
				positions);

		REQUIRE( completion->is_ready() );
		CHECK_NOTHROW( completion->get() );
	}

	SECTION( "an odd extent" )
	{
		REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
			.LR_WITH(
				to_vector(_2.get_extents()) ==
					std::vector<std::size_t>{9, 9} &&
				to_vector(_2.get_file_offset(0)) ==
					std::vector<std::size_t>{46, 46}
			);

		const auto completion =
			read_patches_async(
			*source,
			make_array({1, 9, 9}), image_location("a.mrc"),
				positions);

		REQUIRE( completion->is_ready() );
		CHECK_NOTHROW( completion->get() );
	}
}

TEST_CASE(
	"read_patches_async gives each patch its own slot of the batch",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 3 &&
			_2.get_file_rank() == 2 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{15, 25} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{35, 45} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0} &&
			to_vector(_2.get_file_offset(2)) ==
				std::vector<std::size_t>{55, 65} &&
			to_vector(_2.get_array_offset(2)) ==
				std::vector<std::size_t>{2, 0, 0}
		);

	const auto source = make_source(readers);
	const auto positions =
		make_positions({{20, 30}, {40, 50}, {60, 70}}, 2);

	const auto completion =
		read_patches_async(
			*source,
			make_array({3, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async carries the part of a patch before the image in its "
	"array offset",
	"[image_read]"
)
{
	// A patch of ten centred at three starts two rows before the image
	// begins. The region still names a whole patch, and the downstream
	// source is what shortens it.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{8, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{0, 45} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 2, 0}
		);

	const auto source = make_source(readers);
	const auto positions = make_positions({{3, 50}}, 2);

	const auto completion =
		read_patches_async(
			*source,
			make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async shortens a patch running past the far edge",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{7, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{93, 45} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0}
		);

	const auto source = make_source(readers);
	const auto positions = make_positions({{98, 50}}, 2);

	const auto completion =
		read_patches_async(
			*source,
			make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async cuts the patches of a stack out of one slice",
	"[image_read]"
)
{
	// A location carrying a position in a stack grows the file rank by the
	// axis the stack is indexed along, which every patch of the batch
	// shares since they all come from one image.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("stack.mrcs")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_stack_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 2 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 3 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{10, 10} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{4, 15, 25} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0} &&
			to_vector(_2.get_file_offset(1)) ==
				std::vector<std::size_t>{4, 35, 45} &&
			to_vector(_2.get_array_offset(1)) ==
				std::vector<std::size_t>{1, 0, 0}
		);

	const auto source = make_source(readers);
	const auto positions = make_positions({{20, 30}, {40, 50}}, 2);

	const auto completion =
		read_patches_async(
			*source,
			make_array({2, 10, 10}),
			image_location("stack.mrcs", 4), positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async cuts boxes out of a volume the same way",
	"[image_read]"
)
{
	// Nothing about the class is two dimensional: a subtomogram is a patch
	// of one more axis.
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("tomogram.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(volume_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.LR_WITH(
			_2.get_region_count() == 1 &&
			_2.get_file_rank() == 3 &&
			_2.get_array_rank() == 4 &&
			to_vector(_2.get_extents()) ==
				std::vector<std::size_t>{8, 8, 8} &&
			to_vector(_2.get_file_offset(0)) ==
				std::vector<std::size_t>{16, 26, 36} &&
			to_vector(_2.get_array_offset(0)) ==
				std::vector<std::size_t>{0, 0, 0, 0}
		);

	const auto source = make_source(readers);
	const auto positions = make_positions({{20, 30, 40}}, 3);

	const auto completion =
		read_patches_async(
			*source,
			make_array({1, 8, 8, 8}),
			image_location("tomogram.mrc"), positions);

	REQUIRE( completion->is_ready() );
	CHECK_NOTHROW( completion->get() );
}

TEST_CASE(
	"read_patches_async's completion reports what the source threw",
	"[image_read]"
)
{
	const auto readers = std::make_shared<mock_image_reader_provider>();
	const auto reader = std::make_shared<mock_image_reader>();

	REQUIRE_CALL(*readers, acquire("a.mrc")).RETURN(reader);
	ALLOW_CALL(*reader, get_extents()).RETURN(make_span(patch_image_extents));
	REQUIRE_CALL(*reader, read(trompeloeil::_, trompeloeil::_))
		.SIDE_EFFECT( throw std::runtime_error("from a reader") );

	const auto source = make_source(readers);
	const auto positions = make_positions({{50, 50}}, 2);

	const auto completion =
		read_patches_async(
			*source,
			make_array({1, 10, 10}), image_location("a.mrc"),
			positions);

	REQUIRE( completion->is_ready() );
	REQUIRE_THROWS_AS( completion->get(), std::runtime_error );
}
