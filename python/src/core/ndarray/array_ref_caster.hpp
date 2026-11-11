// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/array_ref.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>

#include <nanobind/nanobind.h>

namespace nanobind
{
namespace detail
{

/**
 * Loads a reference to an array from a bound Array, under the name of
 * Array. It is never cast to Python.
 */
template <typename Ref, typename Referenced>
struct array_ref_caster
{
	NB_TYPE_CASTER(Ref, make_caster<rexlib::array>::Name)

	bool from_python(handle src, uint32_t flags, cleanup_list *cleanup) noexcept
	{
		make_caster<Referenced> caster;
		if (
			!caster.from_python(
				src,
				flags_for_local_caster<Referenced&>(flags),
				cleanup
			) ||
			!caster.template can_cast<Referenced&>()
		)
		{
			return false;
		}

		value = Ref(caster.operator Referenced&());
		return true;
	}
};

template <>
struct type_caster<rexlib::const_array_ref>
	: array_ref_caster<rexlib::const_array_ref, rexlib::array>
{
};

template <>
struct type_caster<rexlib::array_ref>
	: array_ref_caster<rexlib::array_ref, rexlib::array>
{
};

} // namespace detail
} // namespace nanobind
