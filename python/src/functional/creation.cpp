// SPDX-License-Identifier: GPL-3.0-only

#include "creation.hpp"

#include "../core/numerical/float16_caster.hpp"

#include <rexlib/functional/creation.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_descriptor.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>
#include <rexlib/core/numerical/scalar_value.hpp>
#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

#include <pybind11/stl.h>

#include <optional>
#include <string>

namespace rexlib
{

namespace py = pybind11;

// Kept out of the dispatch below, which would otherwise build this message
// once per numerical type. Nothing here depends on the type being cast to.
[[noreturn]]
static void throw_scalar_cast_error(numerical_type type, const py::object &value)
{
	const auto given =
		py::str(py::type::of(value).attr("__name__")).cast<std::string>();
	auto message =
		"cannot use a '" + given + "' as a " + to_string(type) + " value";
	if (type == numerical_type::char8)
	{
		message +=
			"; char8 holds characters, so it takes a "
			"single-character str or bytes";
	}
	throw py::type_error(message);
}

static scalar_value make_scalar_value(numerical_type type, const py::object &value)
{
	return dispatch_numerical_types(
		[&value, type](auto tag)
		{
			using T = typename decltype(tag)::type;
			try
			{
				return scalar_value(value.cast<T>());
			}
			catch (const py::cast_error&)
			{
				throw_scalar_cast_error(type, value);
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
	return rexlib::empty(
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
	return rexlib::zeros(
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
	return rexlib::ones(
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
	return rexlib::full(
		std::move(descriptor), affinity, scalar, context, out.value_or(nullptr)
	);
}

static array py_copy(
	const array &source,
	const execution_context &context,
	std::optional<array*> out
)
{
	return rexlib::copy(source, context, out.value_or(nullptr));
}

static void py_fill(array &out, const py::object &fill_value, const execution_context &context)
{
	const auto scalar = make_scalar_value(out.get_descriptor().get_data_type(), fill_value);
	rexlib::fill(out, scalar, context);
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

} // namespace rexlib
