// SPDX-License-Identifier: GPL-3.0-only

#include "transfer.hpp"

#include <xmipp4/functional/transfer.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/dispatch/execution_context.hpp>

#include <pybind11/stl.h> // Required for std::optional binding

#include <optional>

namespace xmipp4
{
namespace functional
{

namespace py = pybind11;

static array py_transfer_copy(
	array &input,
	memory_resource_affinity affinity,
	const execution_context &context,
	std::optional<array*> out
)
{
	return xmipp4::transfer_copy(input, affinity, context, out.value_or(nullptr));
}

static array py_to_device_copy(array &input, const execution_context &context, std::optional<array*> out)
{
	return xmipp4::to_device_copy(input, context, out.value_or(nullptr));
}

static array py_to_host_copy(array &input, const execution_context &context, std::optional<array*> out)
{
	return xmipp4::to_host_copy(input, context, out.value_or(nullptr));
}

void bind_transfer(pybind11::module_ &m)
{
	m.def(
		"transfer",
		&xmipp4::transfer,
		py::arg("input"), py::arg("affinity"), py::arg("context")
	);
	m.def(
		"transfer_copy", &py_transfer_copy,
		py::arg("input"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = py::none()
	);
	m.def(
		"to_device",
		&xmipp4::to_device,
		py::arg("input"), py::arg("context")
	);
	m.def(
		"to_device_copy", &py_to_device_copy,
		py::arg("input"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"to_host",
		&xmipp4::to_host,
		py::arg("input"), py::arg("context")
	);
	m.def(
		"to_host_copy", &py_to_host_copy,
		py::arg("input"), py::arg("context"), py::arg("out") = py::none()
	);
}

} // namespace functional
} // namespace xmipp4
