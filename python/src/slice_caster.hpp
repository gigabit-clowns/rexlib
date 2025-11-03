// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/slice.hpp>

#include <pybind11/pybind11.h>

namespace PYBIND11_NAMESPACE 
{ 
namespace detail 
{

template <typename Start, typename Stride, typename Stop>
struct type_caster<xmipp4::slice<Start, Stride, Stop>>
{
	using slice_type = xmipp4::slice<Start, Stride, Stop>;

	bool load(handle src, bool) 
	{
		// Check for the correct type
		if (!src)
		{
			return false;
		}
		if (!PySlice_Check(src.ptr())) 
		{
			return false;
		}

		// Extract values
		Py_ssize_t start, stop, step;
		if (PySlice_Unpack(src.ptr(), &start, &stop, &step))
		{
			return false;
		}

		// Write
		if (step < 0)
		{
			start = reverse_index_from_python(start);
			stop = reverse_index_from_python(stop);
		}

		value = xmipp4::make_slice(
			start,
			stop, // Python uses same convention for end signaling
			step
		);

		return true;
	}

	template <typename T>
	static handle cast(T &&src, return_value_policy, handle) 
	{
		const auto start_value = src.get_start();
		const auto stop_value = src.get_stop();
		const auto step_value = src.get_step();

		PyObject* start;
		PyObject* stop;
		PyObject* step = select_default_or_value(step_value, xmipp4::adjacent());
		if (step_value < 0)
		{
			start = reverse_index_to_python(start_value, xmipp4::end());
			stop = reverse_index_to_python(stop_value, xmipp4::begin());
		}
		else
		{
			start = select_default_or_value(start_value, xmipp4::begin());
			stop = select_default_or_value(stop_value, xmipp4::end());
		}

		return PySlice_New(start, stop, step);
	}

	PYBIND11_TYPE_CASTER(slice_type, const_name("slice"));

private:
	template <typename T, typename Q>
	static PyObject* select_default_or_value(T value, Q default_value)
	{
		PyObject* result;
		if(value == default_value)
			result = Py_None;
		else
			result = PyLong_FromLong(static_cast<Py_ssize_t>(value));
		return result;
	}

	static Py_ssize_t reverse_index_from_python(Py_ssize_t value) noexcept
	{
		if (value == std::numeric_limits<Py_ssize_t>::max())
			return value;

		if (value == std::numeric_limits<Py_ssize_t>::min())
			return 0;

		if (value == -1)
			return std::numeric_limits<Py_ssize_t>::max();

		return value + 1;
	}

	template <typename T, typename Q>
	static PyObject* reverse_index_to_python(T value, Q default_value)
	{
		PyObject* result;
		if(value == default_value)
			result = Py_None;
		else
			result = PyLong_FromLong(static_cast<Py_ssize_t>(value) - 1);
		return result;
	}
	
};

} // namespace detail
} // namespace PYBIND11_NAMESPACE
