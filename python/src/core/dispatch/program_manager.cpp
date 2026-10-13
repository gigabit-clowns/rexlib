// SPDX-License-Identifier: GPL-3.0-only

#include "program_manager.hpp"

#include <rexlib/core/service_catalog.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

static std::shared_ptr<program_manager> get_program_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<program_manager>();
}

program_manager_class declare_program_manager(nb::module_ &m)
{
	return program_manager_class(m, "ProgramManager");
}

void define_program_manager(nb::module_ &m)
{
	m.def("get_program_manager", &get_program_manager);
}

} // namespace rexlib
