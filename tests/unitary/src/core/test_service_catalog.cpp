// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/service_catalog.hpp>

#include "mock/mock_service_manager.hpp"

#include <rexlib/core/hardware/device_manager.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using namespace rexlib;

TEST_CASE(
	"service_catalog returns the same instance on repeated access",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	const auto first = catalog.get_service_manager<mock_service_manager>();
	const auto second = catalog.get_service_manager<mock_service_manager>();

	REQUIRE(first);
	CHECK(first == second);
}

TEST_CASE(
	"service_catalog hands each service_manager type its own instance",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	const auto first = catalog.get_service_manager<mock_service_manager>();
	const auto second = catalog.get_service_manager<device_manager>();

	REQUIRE(first);
	REQUIRE(second);
	// Different types must not be aliased under a shared catalog key.
	CHECK(
		static_cast<service_manager *>(first.get())
		!= static_cast<service_manager *>(second.get())
	);
}

TEST_CASE(
	"service_catalog registers the plugins built into the library when "
	"asked to",
	"[service_catalog]"
)
{
	service_catalog catalog(true);
	std::vector<std::string> backends;

	catalog.get_service_manager<device_manager>()->enumerate_backends(backends);

	CHECK(
		std::find(backends.cbegin(), backends.cend(), "cpu")
		!= backends.cend()
	);
}

TEST_CASE(
	"service_catalog registers no built-in plugin when not asked to",
	"[service_catalog]"
)
{
	service_catalog catalog(false);
	std::vector<std::string> backends;

	catalog.get_service_manager<device_manager>()->enumerate_backends(backends);

	CHECK(backends.empty());
}

TEST_CASE(
	"service_catalog preserves managers through move construction",
	"[service_catalog]"
)
{
	service_catalog source(false);
	const auto manager =
		source.get_service_manager<mock_service_manager>();

	service_catalog destination(std::move(source));
	const auto reloaded =
		destination.get_service_manager<mock_service_manager>();

	CHECK(reloaded == manager);
}

TEST_CASE(
	"service_catalog preserves managers through move assignment",
	"[service_catalog]"
)
{
	service_catalog source(false);
	const auto manager =
		source.get_service_manager<mock_service_manager>();

	service_catalog destination(false);
	destination = std::move(source);
	const auto reloaded =
		destination.get_service_manager<mock_service_manager>();

	CHECK(reloaded == manager);
}
