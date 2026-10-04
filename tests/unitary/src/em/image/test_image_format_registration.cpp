// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_format_registration.hpp>

#include "mock/mock_image_format_registry.hpp"
#include "mock/mock_image_read_format.hpp"
#include "mock/mock_image_write_format.hpp"

#include <rexlib/em/image/image_read_format.hpp>
#include <rexlib/em/image/image_write_format.hpp>

#include <trompeloeil.hpp>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"a registration appends a factory for its format to a registry",
	"[image_format_registration]"
)
{
	SECTION( "a read format" )
	{
		using registry_type = mock_image_format_registry<image_read_format>;
		registry_type registry;

		REQUIRE_CALL(registry, add(trompeloeil::_))
			.WITH(
				dynamic_cast<const mock_image_read_format*>(_1().get()) !=
				nullptr
			);

		const image_format_registration<
			mock_image_read_format,
			registry_type
		> registration(registry);
	}

	SECTION( "a write format" )
	{
		using registry_type = mock_image_format_registry<image_write_format>;
		registry_type registry;

		REQUIRE_CALL(registry, add(trompeloeil::_))
			.WITH(
				dynamic_cast<const mock_image_write_format*>(_1().get()) !=
				nullptr
			);

		const image_format_registration<
			mock_image_write_format,
			registry_type
		> registration(registry);
	}
}
