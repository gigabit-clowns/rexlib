// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/hardware/memory_resource_kind.hpp>

namespace rexlib 
{

int score_host_affinity(memory_resource_kind kind) noexcept
{
    switch (kind)
    {
    case memory_resource_kind::host: 
    case memory_resource_kind::host_staging: 
    case memory_resource_kind::device_mapped:
    case memory_resource_kind::unified:
		return 2;
    case memory_resource_kind::managed:
		return 1;
    default:                                 
		return 0;
    }
}

int score_device_affinity(memory_resource_kind kind) noexcept
{
    switch (kind)
    {
    case memory_resource_kind::device_local:  
		return 4;
    case memory_resource_kind::unified:
		return 3;
    case memory_resource_kind::managed:
		return 2;
    case memory_resource_kind::device_mapped:
		return 1;
    default:
		return 0;
    }
}

bool is_host_accessible(memory_resource_kind kind) noexcept
{
	return score_host_affinity(kind) > 0;
}

bool is_device_accessible(memory_resource_kind kind) noexcept
{
	return score_device_affinity(kind) > 0;
}

} // namespace rexlib
