// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <core/builtin_plugin_registration.hpp>

#include "mock/mock_builtin_plugin_registry.hpp"
#include "mock/mock_plugin.hpp"

#include <trompeloeil.hpp>

using namespace rexlib;

TEST_CASE(
	"builtin_plugin_registration adds a new plugin of its type to a registry",
	"[builtin_plugin_registration]"
)
{
	mock_builtin_plugin_registry registry;

	REQUIRE_CALL(registry, add(trompeloeil::_))
		.WITH(dynamic_cast<const mock_plugin*>(_1.get()) != nullptr);

	const builtin_plugin_registration<
		mock_plugin,
		mock_builtin_plugin_registry
	> registration(registry);
}
