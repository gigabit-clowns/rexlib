// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write.hpp>

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include "../../core/hardware/mock/mock_buffer.hpp"
#include "mock/mock_image_writer.hpp"

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
