// SPDX-License-Identifier: GPL-3.0-only

#include "test_fixture.hpp"

#include <rexlib/core/hardware/command_queue.hpp>
#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/hardware/device_backend.hpp>
#include <rexlib/core/hardware/device_manager.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/platform/operating_system.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace cuda
{

cuda_fixture::cuda_fixture()
	: m_devices(m_catalog.get_service_manager<rexlib::device_manager>())
{
	if (!m_manager.load_plugin(get_plugin_path()))
	{
		return;
	}
	register_all_plugins_at(m_manager, m_catalog);

	auto *backend = m_devices->get_backend("cuda");
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

cuda_fixture::~cuda_fixture() = default;

bool cuda_fixture::has_device() const noexcept
{
	return m_device != nullptr;
}

rexlib::device& cuda_fixture::get_device() const noexcept
{
	return *m_device;
}

std::shared_ptr<memory_allocator> cuda_fixture::create_allocator(
	memory_resource_affinity affinity
) const
{
	return m_device->get_memory_resource(affinity).create_allocator();
}

std::shared_ptr<rexlib::command_queue>
cuda_fixture::create_foreign_queue() const
{
	auto *backend = m_devices->get_backend("cpu");
	REQUIRE( backend != nullptr );

	std::vector<std::size_t> ids;
	backend->enumerate_devices(ids);
	REQUIRE_FALSE( ids.empty() );

	return backend->create_device(ids.front())->create_command_queue();
}

std::string cuda_fixture::get_plugin_path()
{
	#if REXLIB_WINDOWS
		return "rexlib-cuda.dll";
	#elif REXLIB_LINUX
		return "./librexlib-cuda.so";
	#elif REXLIB_APPLE
		return "./librexlib-cuda.dylib";
	#else
		#error "Unknown platform"
	#endif
}

} // namespace cuda
} // namespace rexlib
