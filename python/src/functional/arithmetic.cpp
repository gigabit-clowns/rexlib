// SPDX-License-Identifier: GPL-3.0-only

#include "arithmetic.hpp"

#include <xmipp4/functional/arithmetic.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/dispatch/execution_context.hpp>

namespace xmipp4
{
namespace functional
{

namespace py = pybind11;

static array py_add(array &x, array &y, const execution_context &context, array *out)
{
	return xmipp4::add(x, y, context, out);
}

static array py_subtract(array &x, array &y, const execution_context &context, array *out)
{
	return xmipp4::subtract(x, y, context, out);
}

static array py_negate(array &x, const execution_context &context, array *out)
{
	return xmipp4::negate(x, context, out);
}

static array py_abs(array &x, const execution_context &context, array *out)
{
	return xmipp4::abs(x, context, out);
}

static array py_multiply(array &x, array &y, const execution_context &context, array *out)
{
	return xmipp4::multiply(x, y, context, out);
}

static array py_divide(array &x, array &y, const execution_context &context, array *out)
{
	return xmipp4::divide(x, y, context, out);
}

static array py_modulo(array &x, array &y, const execution_context &context, array *out)
{
	return xmipp4::modulo(x, y, context, out);
}

void bind_arithmetic(pybind11::module_ &m)
{
	m.def(
		"add", &py_add,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"subtract", &py_subtract,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"negate", &py_negate,
		py::arg("x"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"abs", &py_abs,
		py::arg("x"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"multiply", &py_multiply,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"divide", &py_divide,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = nullptr
	);
	m.def(
		"modulo", &py_modulo,
		py::arg("x"), py::arg("y"), py::arg("context"), py::arg("out") = nullptr
	);
}

} // namespace functional
} // namespace xmipp4
