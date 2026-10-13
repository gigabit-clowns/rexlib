// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource_kind.hpp"

#include <rexlib/core/hardware/memory_resource_kind.hpp>

namespace rexlib
{

namespace nb = nanobind;

memory_resource_kind_class declare_memory_resource_kind(nb::module_ &m)
{
	return memory_resource_kind_class(m, "MemoryResourceKind");
}

void define_memory_resource_kind(memory_resource_kind_class &c)
{
	c
		.value("unified", memory_resource_kind::unified)
		.value("managed", memory_resource_kind::managed)
		.value("device_local", memory_resource_kind::device_local)
		.value("device_mapped", memory_resource_kind::device_mapped)
		.value("host_staging", memory_resource_kind::host_staging)
		.value("host", memory_resource_kind::host);
}

} // namespace rexlib
