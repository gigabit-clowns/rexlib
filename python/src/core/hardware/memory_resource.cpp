// SPDX-License-Identifier: GPL-3.0-only

#include "memory_resource.hpp"

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/hardware/memory_allocator.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

memory_resource_class declare_memory_resource(nb::module_ &m)
{
	return memory_resource_class(m, "MemoryResource");
}

void define_memory_resource(memory_resource_class &c, nb::module_ &m)
{
	c
		.def_prop_ro("kind", &memory_resource::get_kind)
		.def("create_allocator", &memory_resource::create_allocator);

	m.def(
		"get_host_memory_resource",
		&get_host_memory_resource,
		nb::rv_policy::reference
	);
}

} // namespace rexlib
