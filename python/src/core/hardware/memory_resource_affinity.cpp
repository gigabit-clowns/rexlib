// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource_affinity.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>

namespace rexlib
{

namespace nb = nanobind;

memory_resource_affinity_class declare_memory_resource_affinity(nb::module_ &m)
{
	return memory_resource_affinity_class(m, "MemoryResourceAffinity");
}

void define_memory_resource_affinity(memory_resource_affinity_class &c)
{
	c
		.value("host", memory_resource_affinity::host)
		.value("device", memory_resource_affinity::device);
}

} // namespace rexlib
