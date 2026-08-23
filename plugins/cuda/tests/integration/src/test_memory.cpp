// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/hardware/buffer.hpp>
#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/hardware/device_backend.hpp>
#include <xmipp4/core/hardware/device_manager.hpp>
#include <xmipp4/core/hardware/memory_allocator.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/hardware/memory_resource_kind.hpp>
#include <xmipp4/core/platform/operating_system.h>
#include <xmipp4/core/plugin_manager.hpp>
#include <xmipp4/core/service_catalog.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <cuda_runtime.h>

using namespace xmipp4;

namespace
{

std::string get_cuda_plugin_path()
{
	#if XMIPP4_WINDOWS
		return "xmipp4-cuda.dll";
	#elif XMIPP4_LINUX
		return "./libxmipp4-cuda.so";
	#elif XMIPP4_APPLE
		return "./libxmipp4-cuda.dylib";
	#else
		#error "Unknown platform"
	#endif
}

/// Everything needed to reach a CUDA device through the loaded plugin.
class cuda_fixture
{
public:
	cuda_fixture()
	{
		if (!m_manager.load_plugin(get_cuda_plugin_path()))
		{
			return;
		}
		register_all_plugins_at(m_manager, m_catalog);

		const auto device_manager =
			m_catalog.get_service_manager<xmipp4::device_manager>();
		auto *backend = device_manager->get_backend("cuda");
		if (!backend)
		{
			return;
		}

		std::vector<std::size_t> ids;
		backend->enumerate_devices(ids);
		if (ids.empty())
		{
			return;
		}

		m_device = backend->create_device(ids.front());
	}

	bool has_device() const noexcept
	{
		return m_device != nullptr;
	}

	xmipp4::device& get_device() const noexcept
	{
		return *m_device;
	}

	std::shared_ptr<memory_allocator> create_allocator(
		memory_resource_affinity affinity
	) const
	{
		return m_device->get_memory_resource(affinity).create_allocator();
	}

private:
	plugin_manager m_manager;
	service_catalog m_catalog;
	std::shared_ptr<xmipp4::device> m_device;
};

/// Ask the driver what it thinks a pointer is.
cudaPointerAttributes query(const void *data)
{
	cudaPointerAttributes result{};
	REQUIRE( cudaPointerGetAttributes(&result, data) == cudaSuccess );
	return result;
}

} // namespace

TEST_CASE(
	"a CUDA device should expose a memory resource for each affinity",
	"[cuda][memory]"
)
{
	const cuda_fixture fixture;
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
	const cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	const std::size_t size = 4096;
	const std::size_t alignment = 256;

	SECTION( "device local memory the host cannot address" )
	{
		auto allocator =
			fixture.create_allocator(memory_resource_affinity::device);
		REQUIRE( allocator != nullptr );

		auto buf = allocator->allocate(size, alignment);
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

		auto buf = allocator->allocate(size, alignment);
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
	"a CUDA allocator should hand the same memory out again",
	"[cuda][memory]"
)
{
	const cuda_fixture fixture;
	if (!fixture.has_device())
	{
		SKIP( "No CUDA capable device is available." );
	}

	auto allocator = fixture.create_allocator(memory_resource_affinity::host);
	REQUIRE( allocator != nullptr );

	const std::size_t size = 4096;
	const std::size_t alignment = 256;

	void *first = nullptr;
	{
		auto buf = allocator->allocate(size, alignment);
		first = buf->get_host_ptr();
	}

	// Caching is the whole point: the second request never reaches the driver.
	auto second = allocator->allocate(size, alignment);
	CHECK( second->get_host_ptr() == first );
}

TEST_CASE(
	"a CUDA allocator should say what it can and cannot do",
	"[cuda][memory]"
)
{
	const cuda_fixture fixture;
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

	CHECK_THROWS( allocator->allocate(1024, 2 * max_alignment) );
}
