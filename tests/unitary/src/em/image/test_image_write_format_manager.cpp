// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write_format_manager.hpp>

#include "mock/mock_image_writer.hpp"
#include "mock/mock_image_write_format.hpp"

#include <rexlib/core/exceptions/invalid_operation_error.hpp>
#include <rexlib/em/image/image_format_registry.hpp>
#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

const std::vector<std::size_t> file_extents = {2, 3, 4};

// What one call to open was handed, so that a test can say the manager
// forwarded what it was given rather than ask a test double what it stored.
struct open_record
{
	std::vector<std::size_t> extents;
	std::size_t core_rank = 0;
};

class staged_format final
	: public image_write_format
{
public:
	staged_format(
		std::string name,
		backend_priority suitability,
		std::shared_ptr<open_record> record = nullptr
	)
		: m_name(std::move(name))
		, m_suitability(suitability)
		, m_record(std::move(record))
	{
	}

	std::string get_name() const override
	{
		return m_name;
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return m_suitability;
	}

	std::unique_ptr<image_writer> open(
		const image_probe &,
		span<const std::size_t> extents,
		std::size_t core_rank,
		numerical_type,
		const image_metadata &
	) const override
	{
		if (m_record)
		{
			m_record->extents.assign(extents.begin(), extents.end());
			m_record->core_rank = core_rank;
		}
		return std::make_unique<mock_image_writer>();
	}

private:
	std::string m_name;
	backend_priority m_suitability;
	std::shared_ptr<open_record> m_record;
};

std::unique_ptr<image_write_format> make_staged(
	std::string name,
	backend_priority suitability,
	std::shared_ptr<open_record> record = nullptr
)
{
	return std::make_unique<staged_format>(
		std::move(name),
		suitability,
		std::move(record)
	);
}

} // namespace

TEST_CASE( "an empty write manager recognizes nothing",
	"[image_write_format_manager]" )
{
	const image_write_format_manager manager;

	SECTION( "no format claims a file" )
	{
		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}

	SECTION( "opening reports that nothing is suitable" )
	{
		REQUIRE_THROWS_AS(
			manager.open(
				"absent.mrc",
				make_span(file_extents),
				2,
				numerical_type::int16,
				image_metadata()
			),
			invalid_operation_error
		);
	}
}

TEST_CASE( "the write manager picks the most suitable format",
	"[image_write_format_manager]" )
{
	image_write_format_manager manager;

	SECTION( "the highest priority wins" )
	{
		manager.register_format(
			make_staged("normal", backend_priority::normal));
		manager.register_format(
			make_staged("optimal", backend_priority::optimal));

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "optimal" );
	}

	SECTION( "a file that does not exist yet is still decided on" )
	{
		manager.register_format(
			make_staged("by_extension", backend_priority::normal));
		const image_probe probe("absent.mrc");

		REQUIRE_FALSE( probe.exists() );
		REQUIRE( manager.get_most_suitable_format(probe) != nullptr );
	}

	SECTION( "the chosen format is handed what the file is to be" )
	{
		const auto record = std::make_shared<open_record>();
		manager.register_format(
			make_staged("only", backend_priority::normal, record)
		);

		const auto writer = manager.open(
			"absent.mrc",
			make_span(file_extents),
			2,
			numerical_type::int16,
			image_metadata()
		);

		REQUIRE( writer != nullptr );
		REQUIRE( record->extents == file_extents );
		REQUIRE( record->core_rank == 2 );
	}
}

TEST_CASE( "the write manager refuses a null format",
	"[image_write_format_manager]" )
{
	image_write_format_manager manager;

	REQUIRE_FALSE( manager.register_format(nullptr) );
	REQUIRE( manager.register_format(
		make_staged("real", backend_priority::normal)) );
}

TEST_CASE( "a write registry hands its formats to a manager",
	"[image_write_format_manager]" )
{
	image_write_format_registry registry;
	image_write_format_manager manager;

	registry.add([] () -> std::unique_ptr<image_write_format>
	{
		return make_staged("registered", backend_priority::normal);
	});
	registry.register_all(manager);

	const auto *chosen = manager.get_most_suitable_format(
		image_probe("absent.mrc"));

	REQUIRE( chosen != nullptr );
	REQUIRE( chosen->get_name() == "registered" );
}

TEST_CASE( "image_write_format is mockable", "[image_write_format_manager]" )
{
	mock_image_write_format format;

	REQUIRE_CALL(format, get_suitability(ANY(const image_probe&)))
		.RETURN(backend_priority::unsupported);

	const image_write_format &interface = format;

	REQUIRE( interface.get_suitability(image_probe("absent.mrc")) ==
		backend_priority::unsupported );
}
