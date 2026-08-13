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

#include <pybind11/stl.h> // Required for std::optional binding

#include <optional>
#include <string>

namespace xmipp4
{
namespace functional
{

namespace py = pybind11;

static scalar_value make_scalar_value(numerical_type type, const py::object &value)
{
	return dispatch_numerical_types(
		[&value, type](auto tag) -> scalar_value
		{
			using T = typename decltype(tag)::type;
			try
			{
				return scalar_value(value.cast<T>());
			}
			catch (const py::cast_error&)
			{
				const auto given =
					py::str(py::type::of(value).attr("__name__")).cast<std::string>();
				auto message =
					"cannot use a '" + given + "' as a " + to_string(type) +
					" value";
				if (type == numerical_type::char8)
				{
					message +=
						"; char8 holds characters, so it takes a "
						"single-character str or bytes";
				}
				throw py::type_error(message);
			}
		},
		type
	);
}

static array py_empty(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	std::optional<array*> out
)
{
	return xmipp4::empty(
		std::move(descriptor), affinity, context, out.value_or(nullptr)
	);
}

static array py_zeros(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	std::optional<array*> out
)
{
	return xmipp4::zeros(
		std::move(descriptor), affinity, context, out.value_or(nullptr)
	);
}

static array py_ones(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const execution_context &context,
	std::optional<array*> out
)
{
	return xmipp4::ones(
		std::move(descriptor), affinity, context, out.value_or(nullptr)
	);
}

static array py_full(
	array_descriptor descriptor,
	memory_resource_affinity affinity,
	const py::object &fill_value,
	const execution_context &context,
	std::optional<array*> out
)
{
	const auto scalar = make_scalar_value(descriptor.get_data_type(), fill_value);
	return xmipp4::full(
		std::move(descriptor), affinity, scalar, context, out.value_or(nullptr)
	);
}

static array py_copy(
	array &source,
	const execution_context &context,
	std::optional<array*> out
)
{
	return xmipp4::copy(source, context, out.value_or(nullptr));
}

static void py_fill(array &out, const py::object &fill_value, const execution_context &context)
{
	const auto scalar = make_scalar_value(out.get_descriptor().get_data_type(), fill_value);
	xmipp4::fill(out, scalar, context);
}

void bind_creation(pybind11::module_ &m)
{
	m.def(
		"empty", &py_empty,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = py::none()
	);
	m.def(
		"zeros", &py_zeros,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = py::none()
	);
	m.def(
		"ones", &py_ones,
		py::arg("descriptor"), py::arg("affinity"), py::arg("context"),
		py::arg("out") = py::none()
	);
	m.def(
		"full", &py_full,
		py::arg("descriptor"), py::arg("affinity"), py::arg("fill_value"),
		py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"copy", &py_copy,
		py::arg("source"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"fill", &py_fill,
		py::arg("out"), py::arg("fill_value"), py::arg("context")
	);
}

} // namespace functional
} // namespace xmipp4
