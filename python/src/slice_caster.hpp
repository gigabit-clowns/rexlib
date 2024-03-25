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
	PYBIND11_TYPE_CASTER(slice_type, const_name("slice"));

    bool load(handle src, bool) {
		// Check for the correct type
        if (!PySlice_Check(src.ptr())) {
            return false;
        }

        // Extract values
        Py_ssize_t start, step, stop;
        if (!PySlice_Unpack(src.ptr(), &start, &step, &stop))
        {
            return false;
        }

        // Write
        value.set_start(start);
        value.set_stride(step);
        value.set_stop(stop);

        return true;
    }

    template <typename T>
    static handle cast(T &&src, return_value_policy, handle) {
		return slice(src.get_start(), src.get_stride(), src.get_stop());
    }

};

} // namespace detail
} // namespace PYBIND11_NAMESPACE
