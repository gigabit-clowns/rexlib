// SPDX-License-Identifier: GPL-3.0-only

#include "preprocessing.hpp"

#include "../../core/ndarray/array_ref_caster.hpp"

#include <rexlib/em/functional/preprocessing.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>
#include <rexlib/core/span.hpp>

#include <nanobind/stl/vector.h>

#include <cstddef>
#include <vector>

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

static array py_standardize_background(
	const_array_ref value,
	const_array_ref seed,
	const std::vector<std::ptrdiff_t> &axes,
	double radius,
	double lower_bound,
	double upper_bound,
	const execution_context &context,
	array *out
)
{
	return standardize_background(
		value,
		seed,
		make_span(axes),
		radius,
		lower_bound,
		upper_bound,
		context,
		out
	);
}

void bind_preprocessing(nb::module_ &m)
{
	m.def(
		"standardize_background", &py_standardize_background,
		nb::arg("value"), nb::arg("seed"), nb::arg("axes"),
		nb::arg("radius"), nb::arg("lower_bound"), nb::arg("upper_bound"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"standardize_background_2d", &standardize_background_2d,
		nb::arg("value"), nb::arg("seed"),
		nb::arg("radius"), nb::arg("lower_bound"), nb::arg("upper_bound"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
	m.def(
		"standardize_background_3d", &standardize_background_3d,
		nb::arg("value"), nb::arg("seed"),
		nb::arg("radius"), nb::arg("lower_bound"), nb::arg("upper_bound"),
		nb::arg("context"),
		nb::arg("out").none() = nb::none()
	);
}

} // namespace em
} // namespace rexlib
