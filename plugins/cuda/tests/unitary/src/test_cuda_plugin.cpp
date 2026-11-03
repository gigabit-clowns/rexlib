// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <cuda_plugin.hpp>

#include <hardware/cuda_device_backend.hpp>

#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/service_catalog.hpp>

#include <string>
#include <vector>

using namespace rexlib;

TEST_CASE(
	"a cuda_plugin should be named rexlib-cuda",
	"[plugin]"
)
{
	const cuda::cuda_plugin plugin;

	CHECK( plugin.get_name() == "rexlib-cuda" );
}

TEST_CASE(
	"a cuda_plugin should register the CUDA backend and nothing else",
	"[plugin]"
)
{
	service_catalog catalog(false);
	const cuda::cuda_plugin plugin;

	plugin.register_at(catalog);

	const auto devices = catalog.get_service_manager<device_manager>();
	REQUIRE( devices != nullptr );

	std::vector<std::string> names;
	devices->enumerate_backends(names);
	CHECK( names == std::vector<std::string>{ "cuda" } );

	CHECK(
		dynamic_cast<const cuda::cuda_device_backend*>(
			devices->get_backend("cuda")
		) != nullptr
	);
}
