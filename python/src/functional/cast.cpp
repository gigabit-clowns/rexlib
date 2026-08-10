// SPDX-License-Identifier: GPL-3.0-only

#include "cast.hpp"

#include <xmipp4/functional/cast.hpp>
#include <xmipp4/core/ndarray/array.hpp>
#include <xmipp4/core/numerical/numerical_type.hpp>
#include <xmipp4/core/dispatch/execution_context.hpp>

namespace xmipp4
{
namespace functional
{

namespace py = pybind11;

static array py_cast_copy(
	array &input,
	numerical_type target_type,
	const execution_context &context,
	array *out
)
{
	return xmipp4::cast_copy(input, target_type, context, out);
}

void bind_cast(pybind11::module_ &m)
{
	m.def(
		"cast",
		&xmipp4::cast,
		py::arg("input"), py::arg("target_type"), py::arg("context")
	);
	m.def(
		"cast_copy", &py_cast_copy,
		py::arg("input"), py::arg("target_type"), py::arg("context"),
		py::arg("out") = nullptr
	);
}

} // namespace functional
} // namespace xmipp4
