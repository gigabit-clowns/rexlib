// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write_format_registry.hpp>

#include "mock/mock_image_writer.hpp"

#include <rexlib/em/image/image_metadata.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <cstddef>
#include <memory>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

// A format that claims every file under a name of its own, so that asking a
// manager for a file says whether the manager was handed one.
class claiming_format final
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

std::unique_ptr<image_write_format> make_claiming_format()
{
	return std::make_unique<claiming_format>();
}

} // anonymous namespace

TEST_CASE(
	"a write registry hands its formats to a manager",
	"[image_write_format_registry]"
)
{
	image_write_format_registry registry;
	image_write_format_manager manager;

	SECTION( "a drained registry populates the manager" )
	{
		registry.add(&make_claiming_format);
		registry.register_all(manager);

		const auto *chosen = manager.get_most_suitable_format(
			image_probe("absent.mrc"));

		REQUIRE( chosen != nullptr );
		REQUIRE( chosen->get_name() == "registered" );
	}

	SECTION( "a null factory is ignored" )
	{
		registry.add(nullptr);
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}

	SECTION( "an empty registry registers nothing" )
	{
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(
			image_probe("absent.mrc")) == nullptr );
	}
}
