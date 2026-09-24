// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_write_format_registry.hpp>

#include "mock/mock_image_format_factory.hpp"
#include "mock/mock_image_write_format.hpp"

#include <rexlib/core/backend_priority.hpp>
#include <rexlib/em/image/image_probe.hpp>
#include <rexlib/em/image/image_write_format.hpp>
#include <rexlib/em/image/image_write_format_manager.hpp>

#include <memory>
#include <trompeloeil.hpp>
#include <utility>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"a write registry hands its formats to a manager",
	"[image_write_format_registry]"
)
{
	using factory = mock_image_format_factory<image_write_format>;

	image_write_format_registry registry;
	// Declared before the expectations, so the formats it owns outlive them.
	image_write_format_manager manager;
	const image_probe probe("absent.mrc");

	SECTION( "a drained registry hands the manager what its factory makes" )
	{
		auto format = std::make_unique<mock_image_write_format>();
		const auto *expected = format.get();

		ALLOW_CALL(*format, get_suitability(ANY(const image_probe&)))
			.RETURN(backend_priority::normal);
		REQUIRE_CALL(factory::get_instance(), make())
			.LR_RETURN(std::move(format));

		registry.add(&factory::create);
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(probe) == expected );
	}

	SECTION( "a null factory is ignored" )
	{
		registry.add(nullptr);
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(probe) == nullptr );
	}

	SECTION( "an empty registry registers nothing" )
	{
		registry.register_all(manager);

		REQUIRE( manager.get_most_suitable_format(probe) == nullptr );
	}
}
