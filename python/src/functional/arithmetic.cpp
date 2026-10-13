// SPDX-License-Identifier: GPL-3.0-only

#include "arithmetic.hpp"

#include "../core/ndarray/array_ref_caster.hpp"

#include <rexlib/functional/arithmetic.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

namespace rexlib
{

namespace nb = nanobind;

void bind_arithmetic(nb::module_ &m)
{
	m.def(
		"add", &rexlib::add,
		nb::arg("x"), nb::arg("y"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"subtract", &rexlib::subtract,
		nb::arg("x"), nb::arg("y"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"negate", &rexlib::negate,
		nb::arg("x"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"abs", &rexlib::abs,
		nb::arg("x"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"multiply", &rexlib::multiply,
		nb::arg("x"), nb::arg("y"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"divide", &rexlib::divide,
		nb::arg("x"), nb::arg("y"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"modulo", &rexlib::modulo,
		nb::arg("x"), nb::arg("y"), nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
}

} // namespace rexlib
