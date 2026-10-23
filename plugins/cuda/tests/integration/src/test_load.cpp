// SPDX-License-Identifier: GPL-3.0-only


#include <catch2/catch_test_macros.hpp>

#include "test_fixture.hpp"

#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/device_backend.hpp>
#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/device_type.hpp>
#include <rexlib/core/plugin.hpp>
#include <rexlib/core/plugin_manager.hpp>
#include <rexlib/core/service_catalog.hpp>

#include <algorithm>
#include <stdexcept>

using namespace rexlib;


TEST_CASE( "load and register rexlib-cuda plugin", "[cuda]" )
{
	plugin_manager manager;

	const auto* cuda_plugin =
		manager.load_plugin(cuda::cuda_fixture::get_plugin_path());

	REQUIRE( cuda_plugin != nullptr );
	REQUIRE( cuda_plugin->get_name() == "rexlib-cuda" );
}

TEST_CASE( "register the CUDA device backend", "[cuda]" )
{
	plugin_manager manager;
	REQUIRE(
		manager.load_plugin(cuda::cuda_fixture::get_plugin_path()) != nullptr
	);

	service_catalog catalog;
	register_all_plugins_at(manager, catalog);

	const auto device_manager = catalog.get_service_manager<rexlib::device_manager>();
	REQUIRE( device_manager != nullptr );

	std::vector<std::string> names;
	device_manager->enumerate_backends(names);
	REQUIRE(
		std::find(names.cbegin(), names.cend(), "cuda") != names.cend()
	);

	auto *backend = device_manager->get_backend("cuda");
	REQUIRE( backend != nullptr );
	REQUIRE( backend->get_version().get_major() >= 11 );
}

TEST_CASE( "enumerate the CUDA devices", "[cuda]" )
{
	plugin_manager manager;
	REQUIRE(
		manager.load_plugin(cuda::cuda_fixture::get_plugin_path()) != nullptr
	);

	service_catalog catalog;
	register_all_plugins_at(manager, catalog);

	const auto device_manager = catalog.get_service_manager<rexlib::device_manager>();
	auto *backend = device_manager->get_backend("cuda");
	REQUIRE( backend != nullptr );

	// Enumeration must succeed even when no CUDA capable device or driver is
	// present, which is the case on the CI runners.
	std::vector<std::size_t> ids;
	REQUIRE_NOTHROW( backend->enumerate_devices(ids) );

	for (const auto id : ids)
	{
		device_properties properties;
		REQUIRE( backend->get_device_properties(id, properties) );
		REQUIRE_FALSE( properties.get_name().empty() );
		REQUIRE( properties.get_total_memory_bytes() > 0 );
		REQUIRE(
			(properties.get_type() == device_type::gpu ||
			 properties.get_type() == device_type::integrated_gpu)
		);

		REQUIRE( backend->create_device(id) != nullptr );
	}

	// Unknown ids are rejected regardless of whether there is any device.
	device_properties properties;
	REQUIRE_FALSE( backend->get_device_properties(ids.size(), properties) );
}
