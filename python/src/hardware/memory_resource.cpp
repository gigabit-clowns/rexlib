// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource.hpp"

#include <xmipp4/core/hardware/memory_resource.hpp>
#include <xmipp4/core/hardware/memory_allocator.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

memory_resource_class declare_memory_resource(pybind11::module_ &m)
{
	return memory_resource_class(m, "MemoryResource");
}

void define_memory_resource(memory_resource_class &c, pybind11::module_ &m)
{
	c
		.def_property_readonly("kind", &memory_resource::get_kind)
		.def("create_allocator", &memory_resource::create_allocator);

	m.def(
		"get_host_memory_resource",
		&get_host_memory_resource,
		py::return_value_policy::reference
	);
}

} // namespace hardware
} // namespace xmipp4
