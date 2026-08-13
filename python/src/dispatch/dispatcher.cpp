// SPDX-License-Identifier: GPL-3.0-only

#include "dispatcher.hpp"

#include <xmipp4/core/dispatch/program_manager.hpp>

namespace xmipp4
{
namespace dispatch
{

namespace py = pybind11;

dispatcher_class declare_dispatcher(pybind11::module_ &m)
{
	return dispatcher_class(m, "Dispatcher");
}

void define_dispatcher(pybind11::module_ &m)
{
	m.def(
		"make_eager_dispatcher",
		&make_eager_dispatcher,
		py::arg("program_manager")
	);
}

} // namespace dispatch
} // namespace xmipp4
