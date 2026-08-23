// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/hardware/memory_resource.hpp>

#include <xmipp4/core/hardware/device.hpp>
#include <xmipp4/core/platform/constexpr.hpp>

#include "host_memory/host_memory_resource.hpp"

namespace rex
{

memory_resource::memory_resource() noexcept = default;
memory_resource::~memory_resource() = default;

const memory_resource& get_host_memory_resource() noexcept
{
	return host_memory_resource::get();
}

} // namespace rex
