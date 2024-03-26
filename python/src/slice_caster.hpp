#pragma once

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#include <xmipp4/slice.hpp>

#include <pybind11/pybind11.h>

namespace PYBIND11_NAMESPACE 
{ 
namespace detail 
{

template <typename Start, typename Stride, typename Stop>
struct type_caster<xmipp4::slice<Start, Stride, Stop>>
{
    using slice_type = xmipp4::slice<Start, Stride, Stop>;

    bool load(handle src, bool) {
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
        value = xmipp4::make_slice(
            start,
            step,
            stop // Python uses same convention for end signaling
        );

        return true;
    }

    template <typename T>
    static handle cast(T &&src, return_value_policy, handle) {
        const auto start_value = src.get_start();
        const auto step_value = src.get_stride();
        const auto stop_value = src.get_stop();

        PyObject* start = PyLong_FromLong(start_value);
        PyObject* step = PyLong_FromLong(step_value);
        PyObject* stop = stop_value==end() ? Py_None : PyLong_FromLong(stop_value);
        return PySlice_New(start, stop, step);
    }

	PYBIND11_TYPE_CASTER(slice_type, const_name("slice"));

};

} // namespace detail
} // namespace PYBIND11_NAMESPACE
