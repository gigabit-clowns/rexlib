// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>

namespace pybind11
{
namespace detail
{

// rexlib::float16_t has no native Python counterpart (Python has no
// half-width float type); convert through `float`. This also makes
// std::complex<float16_t> work via pybind11/complex.h, which composes
// on top of type_caster<float16_t> for the real/imaginary parts.
template <>
struct type_caster<rexlib::float16_t>
{
	PYBIND11_TYPE_CASTER(rexlib::float16_t, const_name("float16_t"));

	bool load(handle src, bool convert)
	{
		if (!convert && !PyFloat_Check(src.ptr()) && !PyLong_Check(src.ptr()))
		{
			return false;
		}

		PyObject *tmp = PyNumber_Float(src.ptr());
		if (!tmp)
		{
			PyErr_Clear();
			return false;
		}

		value = rexlib::float16_t(static_cast<float>(PyFloat_AsDouble(tmp)));
		Py_DECREF(tmp);
		return true;
	}

	static handle cast(rexlib::float16_t src, return_value_policy, handle)
	{
		return PyFloat_FromDouble(static_cast<double>(static_cast<float>(src)));
	}
};

} // namespace detail
} // namespace pybind11
