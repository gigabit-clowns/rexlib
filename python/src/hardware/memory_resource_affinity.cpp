// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource_affinity.hpp"

#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

memory_resource_affinity_class declare_memory_resource_affinity(pybind11::module_ &m)
{
	return memory_resource_affinity_class(m, "MemoryResourceAffinity");
}

void define_memory_resource_affinity(memory_resource_affinity_class &c)
{
	c
		.value("host", memory_resource_affinity::host)
		.value("device", memory_resource_affinity::device);
}

} // namespace hardware
} // namespace xmipp4
