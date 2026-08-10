// SPDX-License-Identifier: GPL-3.0-only

#include "event.hpp"

#include <xmipp4/core/hardware/event.hpp>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

void bind_event(pybind11::module_ &m)
{
	py::class_<event, std::shared_ptr<event>>(m, "Event")
		.def_property_readonly("supported_usage", &event::get_supported_usage)
		.def("wait", &event::wait)
		.def_property_readonly("is_signaled", &event::is_signaled);
}

} // namespace hardware
} // namespace xmipp4
