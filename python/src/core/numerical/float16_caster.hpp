// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/numerical/fixed_width_float.hpp>

#include <nanobind/nanobind.h>

namespace nanobind
{
namespace detail
{

// Converts rexlib::float16_t through `float`.
template <>
struct type_caster<rexlib::float16_t>
{
	NB_TYPE_CASTER(rexlib::float16_t, const_name("float"))

	bool from_python(handle src, uint32_t flags, cleanup_list*) noexcept
	{
		const bool convert = (flags & (uint32_t) cast_flags::convert) != 0;
		if (!convert && !PyFloat_Check(src.ptr()) && !PyLong_Check(src.ptr()))
		{
			return false;
		}

		const double result = PyFloat_AsDouble(src.ptr());
		if (result == -1.0 && PyErr_Occurred())
		{
			PyErr_Clear();
			return false;
		}

		value = rexlib::float16_t(static_cast<float>(result));
		return true;
	}

	static handle from_cpp(
		rexlib::float16_t src,
		rv_policy,
		cleanup_list*
	) noexcept
	{
		return PyFloat_FromDouble(static_cast<double>(static_cast<float>(src)));
	}
};

} // namespace detail
} // namespace nanobind
