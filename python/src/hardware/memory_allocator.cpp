// SPDX-License-Identifier: GPL-3.0-only

#include "memory_allocator.hpp"

#include <xmipp4/core/hardware/memory_allocator.hpp>
#include <xmipp4/core/hardware/memory_resource.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

memory_allocator_class declare_memory_allocator(pybind11::module_ &m)
{
	return memory_allocator_class(m, "MemoryAllocator");
}

void define_memory_allocator(memory_allocator_class &c)
{
	c
		.def_property_readonly(
			"memory_resource",
			&memory_allocator::get_memory_resource,
			py::return_value_policy::reference_internal
		)
		.def_property_readonly("max_alignment", &memory_allocator::get_max_alignment);
}

} // namespace hardware
} // namespace xmipp4
