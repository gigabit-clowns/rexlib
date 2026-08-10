// SPDX-License-Identifier: GPL-3.0-only

#include "program_manager.hpp"

#include <xmipp4/core/dispatch/program_manager.hpp>
#include <xmipp4/core/service_catalog.hpp>

namespace xmipp4
{
namespace dispatch
{

static std::shared_ptr<program_manager> get_program_manager(service_catalog &catalog)
{
	return catalog.get_service_manager<program_manager>();
}

namespace py = pybind11;

void bind_program_manager(pybind11::module_ &m)
{
	py::class_<program_manager, std::shared_ptr<program_manager>>(m, "ProgramManager");

	m.def("get_program_manager", &get_program_manager);
}

} // namespace dispatch
} // namespace xmipp4
