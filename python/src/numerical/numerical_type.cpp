// SPDX-License-Identifier: GPL-3.0-only

#include "numerical_type.hpp"

#include <xmipp4/core/numerical/numerical_type.hpp>

namespace xmipp4
{
namespace numerical
{

namespace py = pybind11;

void bind_numerical_type(pybind11::module_ &m)
{
	py::enum_<numerical_type>(m, "NumericalType")
		.value("boolean", numerical_type::boolean)
		.value("char8", numerical_type::char8)
		.value("int8", numerical_type::int8)
		.value("uint8", numerical_type::uint8)
		.value("int16", numerical_type::int16)
		.value("uint16", numerical_type::uint16)
		.value("int32", numerical_type::int32)
		.value("uint32", numerical_type::uint32)
		.value("int64", numerical_type::int64)
		.value("uint64", numerical_type::uint64)
		.value("float16", numerical_type::float16)
		.value("float32", numerical_type::float32)
		.value("float64", numerical_type::float64)
		.value("complex_float16", numerical_type::complex_float16)
		.value("complex_float32", numerical_type::complex_float32)
		.value("complex_float64", numerical_type::complex_float64);
}

} // namespace numerical
} // namespace xmipp4
