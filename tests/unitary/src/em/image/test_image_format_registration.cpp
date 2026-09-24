// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_format_registration.hpp>

#include "mock/mock_image_reader.hpp"
#include "mock/mock_image_writer.hpp"

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_read_format_manager.hpp>
#include <rexlib/em/image/image_read_format_registry.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>
#include <rexlib/em/image/image_write_format_registry.hpp>

#include <cstddef>
#include <memory>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// Formats that claim every file under a name of their own, default
// constructible as a registration requires, so that asking a manager for a
// file says whether the manager was handed one.
class claiming_read_format final
	: public image_read_format
{
public:
	std::string get_name() const override
	{
		return "registered";
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return backend_priority::normal;
	}

	std::shared_ptr<image_reader> open(const image_probe &) const override
	{
		return std::make_shared<mock_image_reader>();
	}
};

class claiming_write_format final
	: public image_write_format
{
public:
	std::string get_name() const override
	{
		return "registered";
	}

	backend_priority get_suitability(const image_probe &) const override
	{
		return backend_priority::normal;
	}

	std::shared_ptr<image_writer> open(
		const image_probe &,
		span<const std::size_t>,
		std::size_t,
		numerical_type,
		const image_metadata &
	) const override
	{
		return std::make_shared<mock_image_writer>();
	}
};

} // anonymous namespace

TEST_CASE(
	"a registration appends a factory for its format to a registry",
	"[image_format_registration]"
)
{
	SECTION( "a read registry" )
	{
		image_read_format_registry registry;
		const image_format_registration<
			claiming_read_format,
			image_read_format_registry
		> registration(registry);

		image_read_format_manager manager;
		registry.register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "registered" );
	}

	SECTION( "a write registry" )
	{
		image_write_format_registry registry;
		const image_format_registration<
			claiming_write_format,
			image_write_format_registry
		> registration(registry);

		image_write_format_manager manager;
		registry.register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "registered" );
	}
}
