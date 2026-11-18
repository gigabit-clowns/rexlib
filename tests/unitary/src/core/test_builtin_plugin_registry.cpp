// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <core/builtin_plugin_registry.hpp>

#include "mock/mock_plugin.hpp"

#include <rexlib/core/service_catalog.hpp>

#include <trompeloeil.hpp>

#include <memory>

using namespace rexlib;

TEST_CASE(
	"builtin_plugin_registry registers every plugin added at a catalog",
	"[builtin_plugin_registry]"
)
{
	builtin_plugin_registry registry;
	auto first = std::make_unique<mock_plugin>();
	auto second = std::make_unique<mock_plugin>();
	service_catalog catalog(false);

	REQUIRE_CALL(*first, register_at(trompeloeil::_))
		.LR_WITH(&_1 == &catalog);
	REQUIRE_CALL(*second, register_at(trompeloeil::_))
		.LR_WITH(&_1 == &catalog);

	registry.add(std::move(first));
	registry.add(std::move(second));
	registry.register_all(catalog);
}

TEST_CASE(
	"builtin_plugin_registry registers nothing while it holds no plugin",
	"[builtin_plugin_registry]"
)
{
	const builtin_plugin_registry registry;
	service_catalog catalog(false);

	registry.register_all(catalog);
}

TEST_CASE(
	"get_builtin_plugin_registry returns the same registry every time",
	"[builtin_plugin_registry]"
)
{
	CHECK( &get_builtin_plugin_registry() == &get_builtin_plugin_registry() );
}
