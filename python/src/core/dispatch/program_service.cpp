// SPDX-License-Identifier: GPL-3.0-only

#include "program_service.hpp"

#include <rexlib/core/service_catalog.hpp>

#include <nanobind/stl/shared_ptr.h>

namespace rexlib
{

namespace nb = nanobind;

static std::shared_ptr<program_service> get_program_service(
	service_catalog &catalog
)
{
	return catalog.get_service<program_service>();
}

program_service_class declare_program_service(nb::module_ &m)
{
	return program_service_class(m, "ProgramService");
}

void define_program_service(nb::module_ &m)
{
	m.def("get_program_service", &get_program_service);
}

} // namespace rexlib
