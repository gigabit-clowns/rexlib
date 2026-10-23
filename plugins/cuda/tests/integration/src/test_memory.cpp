// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "test_fixture.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/command_token.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/hardware/memory_resource_kind.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>

#include <cuda_runtime.h>

using namespace rexlib;

namespace
{

/// Ask the driver what it thinks a pointer is.
cudaPointerAttributes query(const void *data)
{
	cudaPointerAttributes result{};
	REQUIRE( cudaPointerGetAttributes(&result, data) == cudaSuccess );
	return result;
}

} // anonymous namespace

TEST_CASE(
	"a CUDA device should expose a memory resource for each affinity",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto &device_memory =
		fixture.get_device().get_memory_resource(
			memory_resource_affinity::device
		);
	const auto &host_memory =
		fixture.get_device().get_memory_resource(
			memory_resource_affinity::host
		);

	CHECK( is_device_accessible(device_memory.get_kind()) );
	CHECK( is_host_accessible(host_memory.get_kind()) );

	// A device that shares its memory with the host has nothing local to it,
	// and answers both with the one resource. Any other device has two.
	if (device_memory.get_kind() == memory_resource_kind::unified)
	{
		CHECK( &device_memory == &host_memory );
	}
	else
	{
		CHECK( &device_memory != &host_memory );
		CHECK( device_memory.get_kind() == memory_resource_kind::device_local );
	}
}

TEST_CASE(
	"a CUDA device should hand out memory the driver recognizes",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const std::size_t size = 4096;
	const std::size_t alignment = 256;
	command_token pending;

	SECTION( "device local memory the host cannot address" )
	{
		auto allocator =
			fixture.create_allocator(memory_resource_affinity::device);
		REQUIRE( allocator != nullptr );

		auto buf = allocator->allocate(size, alignment, nullptr, pending);
		REQUIRE( buf != nullptr );
		CHECK( buf->get_size() >= size );

		// Nothing on this side of the bus can read it, so offering a pointer
		// would be offering one that must not be dereferenced.
		CHECK( buf->get_host_ptr() == nullptr );
	}

	SECTION( "page locked host memory the driver knows is page locked" )
	{
		auto allocator =
			fixture.create_allocator(memory_resource_affinity::host);
		REQUIRE( allocator != nullptr );

		auto buf = allocator->allocate(size, alignment, nullptr, pending);
		REQUIRE( buf != nullptr );
		CHECK( buf->get_size() >= size );

		auto *data = buf->get_host_ptr();
		REQUIRE( data != nullptr );
		CHECK( reinterpret_cast<std::uintptr_t>(data) % alignment == 0 );

		// Ordinary host memory would come back as unregistered, and would not
		// be transferable without the driver copying it twice.
		const auto attributes = query(data);
		CHECK( attributes.type == cudaMemoryTypeHost );

		// Writing through it is what it is for.
		auto *bytes = static_cast<unsigned char*>(data);
		bytes[0] = 42;
		bytes[size - 1] = 17;
		CHECK( bytes[0] == 42 );
		CHECK( bytes[size - 1] == 17 );
	}
}

TEST_CASE(
	"a CUDA allocator should hand out new memory with nothing to wait for",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const auto affinity = GENERATE(
		memory_resource_affinity::device,
		memory_resource_affinity::host
	);
	auto allocator = fixture.create_allocator(affinity);
	REQUIRE( allocator != nullptr );

	command_token pending;
	auto buf = allocator->allocate(4096, 256, nullptr, pending);

	REQUIRE( buf != nullptr );
	CHECK( pending.is_empty() );
}

TEST_CASE(
	"a CUDA allocator should allocate whatever queue it is hinted",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	auto allocator = fixture.create_allocator(memory_resource_affinity::device);
	REQUIRE( allocator != nullptr );
	command_token pending;

	SECTION( "a queue of its device" )
	{
		const auto queue = fixture.get_device().create_command_queue();

		auto buf = allocator->allocate(4096, 256, queue.get(), pending);

		REQUIRE( buf != nullptr );
		CHECK( pending.is_empty() );
	}

	SECTION( "a queue of another backend" )
	{
		// The hint only says where the buffer is expected to be used first.
		const auto queue = fixture.create_foreign_queue();

		auto buf = allocator->allocate(4096, 256, queue.get(), pending);

		REQUIRE( buf != nullptr );
		CHECK( pending.is_empty() );
	}
}

TEST_CASE(
	"a CUDA allocator should hand the same memory out again",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	auto allocator = fixture.create_allocator(memory_resource_affinity::host);
	REQUIRE( allocator != nullptr );

	const std::size_t size = 4096;
	const std::size_t alignment = 256;
	command_token pending;

	void *first = nullptr;
	{
		auto buf = allocator->allocate(size, alignment, nullptr, pending);
		first = buf->get_host_ptr();
	}

	// Caching is the whole point: the second request never reaches the driver.
	auto second = allocator->allocate(size, alignment, nullptr, pending);
	CHECK( second->get_host_ptr() == first );
}

TEST_CASE(
	"a CUDA allocator should say what it can and cannot do",
	"[cuda][memory]"
)
{
	const cuda::cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	auto allocator = fixture.create_allocator(memory_resource_affinity::device);
	REQUIRE( allocator != nullptr );

	const auto max_alignment = allocator->get_max_alignment();
	CHECK( max_alignment > 0 );
	CHECK( (max_alignment & (max_alignment - 1)) == 0 );

	CHECK(
		&allocator->get_memory_resource() ==
		&fixture.get_device().get_memory_resource(
			memory_resource_affinity::device
		)
	);

	command_token pending;
	CHECK_THROWS(
		allocator->allocate(1024, 2 * max_alignment, nullptr, pending)
	);
}
