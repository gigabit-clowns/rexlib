// SPDX-License-Identifier: GPL-3.0-only

#include "dispatcher.hpp"

#include <xmipp4/core/dispatch/dispatcher.hpp>
#include <xmipp4/core/dispatch/program_manager.hpp>

namespace xmipp4
{
namespace dispatch
{

namespace py = pybind11;

void bind_dispatcher(pybind11::module_ &m)
{
	py::class_<dispatcher, std::shared_ptr<dispatcher>>(m, "Dispatcher");

	m.def(
		"make_eager_dispatcher",
		&make_eager_dispatcher,
		py::arg("program_manager")
	);
}

} // namespace dispatch
} // namespace xmipp4
