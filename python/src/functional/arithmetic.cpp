// SPDX-License-Identifier: GPL-3.0-only

#include "arithmetic.hpp"

#include <rexlib/functional/arithmetic.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

#include <pybind11/stl.h>

#include <optional>

namespace rexlib
{

namespace py = pybind11;

static array py_add(const array &x, const array &y, const execution_context &context, std::optional<array*> out)
{
	return rexlib::add(x, y, context, out.value_or(nullptr));
}

static array py_subtract(const array &x, const array &y, const execution_context &context, std::optional<array*> out)
{
	return rexlib::subtract(x, y, context, out.value_or(nullptr));
}

static array py_negate(const array &x, const execution_context &context, std::optional<array*> out)
{
	return rexlib::negate(x, context, out.value_or(nullptr));
}

static array py_abs(const array &x, const execution_context &context, std::optional<array*> out)
{
	return rexlib::abs(x, context, out.value_or(nullptr));
}

static array py_multiply(const array &x, const array &y, const execution_context &context, std::optional<array*> out)
{
	return rexlib::multiply(x, y, context, out.value_or(nullptr));
}

static array py_divide(const array &x, const array &y, const execution_context &context, std::optional<array*> out)
{
	return rexlib::divide(x, y, context, out.value_or(nullptr));
}

static array py_modulo(const array &x, const array &y, const execution_context &context, std::optional<array*> out)
{
	return rexlib::modulo(x, y, context, out.value_or(nullptr));
}

void bind_arithmetic(pybind11::module_ &m)
{
	m.def(
		"add", &py_add,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"subtract", &py_subtract,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"negate", &py_negate,
		py::arg("x"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"abs", &py_abs,
		py::arg("x"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"multiply", &py_multiply,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"divide", &py_divide,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = py::none()
	);
	m.def(
		"modulo", &py_modulo,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = py::none()
	);
}

} // namespace rexlib
