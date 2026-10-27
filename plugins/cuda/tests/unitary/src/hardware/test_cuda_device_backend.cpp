// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/cuda_device_backend.hpp>

#include <hardware/cuda_device.hpp>

#include "test_driver.hpp"

#include <config.hpp>

#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/hardware/device_properties.hpp>
#include <rexlib/core/hardware/device_type.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <cuda_runtime.h>

using namespace rexlib;

namespace
{

std::size_t get_runtime_device_count() noexcept
{
	int count = 0;
	if (cudaGetDeviceCount(&count) != cudaSuccess)
	{
		count = 0;
	}

	return static_cast<std::size_t>(count);
}

} // anonymous namespace

TEST_CASE(
	"a cuda_device_backend should be named cuda",
	"[device_backend]"
)
{
	const cuda::cuda_device_backend backend;

	CHECK( backend.get_name() == "cuda" );
}

TEST_CASE(
	"a cuda_device_backend should report the major version of the runtime",
	"[device_backend]"
)
{
	const cuda::cuda_device_backend backend;

	constexpr auto runtime_major =
		static_cast<std::uint32_t>(CUDART_VERSION / 1000);
	CHECK( backend.get_version().get_major() == runtime_major );
}

TEST_CASE(
	"a cuda_device_backend should enumerate every device the runtime counts",
	"[device_backend]"
)
{
	const cuda::cuda_device_backend backend;

	std::vector<std::size_t> ids = { 42, 43 };
	backend.enumerate_devices(ids);

	REQUIRE( ids.size() == get_runtime_device_count() );
	for (std::size_t i = 0; i < ids.size(); ++i)
	{
		CHECK( ids[i] == i );
	}
}

TEST_CASE(
	"a cuda_device_backend should not describe a device it does not have",
	"[device_backend]"
)
{
	const cuda::cuda_device_backend backend;

	device_properties properties;
	CHECK_FALSE(
		backend.get_device_properties(get_runtime_device_count(), properties)
	);
}

TEST_CASE(
	"a cuda_device_backend should refuse to create a device it does not have",
	"[device_backend]"
)
{
	const cuda::cuda_device_backend backend;

	CHECK_THROWS_AS(
		backend.create_device(get_runtime_device_count()),
		std::invalid_argument
	);
}

TEST_CASE(
	"a cuda_device_backend should describe the devices it has",
	"[device_backend]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::cuda_device_backend backend;

	device_properties properties;
	REQUIRE( backend.get_device_properties(0, properties) );

	CHECK_FALSE( properties.get_name().empty() );
	CHECK_FALSE( properties.get_physical_location().empty() );
	CHECK(
		(properties.get_type() == device_type::gpu ||
		 properties.get_type() == device_type::integrated_gpu)
	);
	CHECK( properties.get_total_memory_bytes() > 0 );
	CHECK(
		properties.get_optimal_data_alignment() ==
			REXLIB_CUDA_COALESCE_ALIGN_BYTES
	);
}

TEST_CASE(
	"a cuda_device_backend should create the devices it has",
	"[device_backend]"
)
{
	if (!cuda::has_test_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const cuda::cuda_device_backend backend;

	const auto dev = backend.create_device(0);

	REQUIRE( dev != nullptr );
	const auto *cuda_dev = dynamic_cast<const cuda::cuda_device*>(dev.get());
	REQUIRE( cuda_dev != nullptr );
	CHECK( cuda_dev->get_ordinal() == 0 );
}

TEST_CASE(
	"a cuda_device_backend should register itself once at a device manager",
	"[device_backend]"
)
{
	device_manager manager;

	REQUIRE( cuda::cuda_device_backend::register_at(manager) );

	const auto *backend = manager.get_backend("cuda");
	REQUIRE( backend != nullptr );
	CHECK(
		dynamic_cast<const cuda::cuda_device_backend*>(backend) != nullptr
	);

	CHECK_FALSE( cuda::cuda_device_backend::register_at(manager) );
}
