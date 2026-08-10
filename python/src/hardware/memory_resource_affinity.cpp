// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource_affinity.hpp"

#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_memory_resource_affinity(pybind11::module_ &m)
{
	py::enum_<memory_resource_affinity>(m, "MemoryResourceAffinity")
		.value("host", memory_resource_affinity::host)
		.value("device", memory_resource_affinity::device);
}

} // namespace hardware
} // namespace xmipp4
