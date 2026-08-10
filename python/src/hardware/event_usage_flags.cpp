// SPDX-License-Identifier: GPL-3.0-only

#include "event_usage_flags.hpp"

#include <xmipp4/core/hardware/event_usage_flags.hpp>

#include <pybind11/operators.h>

#include <sstream>

namespace xmipp4
{
namespace hardware
{

namespace py = pybind11;

static std::string to_repr(const event_usage_flags &f)
{
	std::ostringstream oss;
	oss << "EventUsageFlags(bits=" << f.get_bits() << ")";
	return oss.str();
}

void bind_event_usage_flags(pybind11::module_ &m)
{
	py::enum_<event_usage_flag_bits>(m, "EventUsageFlagBits")
		.value("host_query", event_usage_flag_bits::host_query)
		.value("host_wait", event_usage_flag_bits::host_wait)
		.value("device_wait", event_usage_flag_bits::device_wait)
		.value("cross_device_wait", event_usage_flag_bits::cross_device_wait);

	py::class_<event_usage_flags>(m, "EventUsageFlags")
		.def(py::init<event_usage_flag_bits>())
		.def(py::init<event_usage_flags::underlying_type>())
		.def(py::self | py::self)
		.def(py::self & py::self)
		.def(py::self ^ py::self)
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def("contains", &event_usage_flags::contains)
		.def_property_readonly("bits", &event_usage_flags::get_bits)
		.def("__repr__", &to_repr);
}

} // namespace hardware
} // namespace xmipp4
