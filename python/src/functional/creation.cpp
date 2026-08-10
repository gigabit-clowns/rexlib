// SPDX-License-Identifier: GPL-3.0-only

#include "creation.hpp"

#include "../numerical/float16_caster.hpp"

#include <xmipp4/functional/creation.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/ndarray/array_descriptor.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/numerical/numerical_type_dispatch.hpp>
#include <xmipp4/core/numerical/scalar_value.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>
#include <xmipp4/core/dispatch/execution_context.hpp>

namespace xmipp4
{
namespace functional
{

namespace py = pybind11;

static scalar_value make_scalar_value(numerical_type type, const py::object &value)
{
	return dispatch_numerical_types(
		[&value](auto tag) -> scalar_value
		{
			using T = typename decltype(tag)::type;
			return scalar_value(value.cast<T>());
		},
		type
	);
}

static array py_full(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const py::object &fill_value,
	const execution_context &context,
	array *out
)
{
	const auto scalar = make_scalar_value(descriptor.get_data_type(), fill_value);
	return xmipp4::full(std::move(descriptor), affinity, scalar, context, out);
}

static array py_copy(array &source, const execution_context &context, array *out)
{
	return xmipp4::copy(source, context, out);
}

static void py_fill(array &out, const py::object &fill_value, const execution_context &context)
{
	const auto scalar = make_scalar_value(out.get_descriptor().get_data_type(), fill_value);
	xmipp4::fill(out, scalar, context);
}

void bind_creation(pybind11::module_ &m)
{
	m.def(
		"empty",
		&xmipp4::empty,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = nullptr
	);
	m.def(
		"zeros",
		&xmipp4::zeros,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = nullptr
	);
	m.def(
		"ones",
		&xmipp4::ones,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = nullptr
	);
	m.def(
		"full", &py_full,
		py::arg("descriptor"), py::arg("affinity"), py::arg("fill_value"),
		py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"copy", &py_copy,
		py::arg("source"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"fill", &py_fill,
		py::arg("out"), py::arg("fill_value"), py::arg("context")
	);
}

} // namespace functional
} // namespace xmipp4
