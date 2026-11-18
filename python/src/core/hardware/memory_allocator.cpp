// SPDX-License-Identifier: GPL-3.0-only

#include "memory_allocator.hpp"

#include <rexlib/core/hardware/memory_allocator.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>

namespace rexlib
{

namespace nb = nanobind;

memory_allocator_class declare_memory_allocator(nb::module_ &m)
{
	return memory_allocator_class(m, "MemoryAllocator");
}

void define_memory_allocator(memory_allocator_class &c)
{
	c
		.def_prop_ro(
			"memory_resource",
			&memory_allocator::get_memory_resource,
			nb::rv_policy::reference_internal
		)
		.def_prop_ro("max_alignment", &memory_allocator::get_max_alignment);
}

} // namespace rexlib
