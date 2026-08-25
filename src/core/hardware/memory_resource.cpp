// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/memory_resource.hpp>

#include <rexlib/core/hardware/device.hpp>
#include <rexlib/core/platform/constexpr.hpp>

#include "host_memory/host_memory_resource.hpp"

namespace rexlib
{

memory_resource::memory_resource() noexcept = default;
memory_resource::~memory_resource() = default;

const memory_resource& get_host_memory_resource() noexcept
{
	return host_memory_resource::get();
}

} // namespace rexlib
