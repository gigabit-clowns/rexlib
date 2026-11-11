// SPDX-License-Identifier: GPL-3.0-only

#include "cast.hpp"

#include "../core/ndarray/array_ref_caster.hpp"

#include <rexlib/functional/cast.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

namespace rexlib
{

namespace nb = nanobind;

void bind_cast(nb::module_ &m)
{
	m.def(
		"cast", &rexlib::cast,
		nb::arg("input"), nb::arg("target_type"), nb::arg("context")
	);
	m.def(
		"cast_copy", &rexlib::cast_copy,
		nb::arg("input"), nb::arg("target_type"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
}

} // namespace rexlib
