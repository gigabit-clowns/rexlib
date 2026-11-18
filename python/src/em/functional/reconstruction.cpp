// SPDX-License-Identifier: GPL-3.0-only

#include "reconstruction.hpp"

#include "../../core/ndarray/array_ref_caster.hpp"

#include <rexlib/em/functional/reconstruction.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/dispatch/execution_context.hpp>

#include <nanobind/stl/pair.h>

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_reconstruction(nb::module_ &m)
{
	m.def(
		"insert_slices", &insert_slices,
		nb::arg("numerator"), nb::arg("denominator"),
		nb::arg("slices"), nb::arg("transforms"), nb::arg("radial_weights"),
		nb::arg("window"),
		nb::arg("context"),
		nb::arg("numerator_out").none() = nb::none(),
		nb::arg("denominator_out").none() = nb::none()
	);
	m.def(
		"insert_weighted_slices", &insert_weighted_slices,
		nb::arg("numerator"), nb::arg("denominator"),
		nb::arg("slices"), nb::arg("weights"), nb::arg("transforms"),
		nb::arg("radial_weights"),
		nb::arg("window"),
		nb::arg("context"),
		nb::arg("numerator_out").none() = nb::none(),
		nb::arg("denominator_out").none() = nb::none()
	);
	m.def(
		"insert_ctf_slices", &insert_ctf_slices,
		nb::arg("numerator"), nb::arg("denominator"),
		nb::arg("slices"), nb::arg("transforms"), nb::arg("ctf"),
		nb::arg("radial_weights"),
		nb::arg("window"), nb::arg("optics"), nb::arg("pixel_size"),
		nb::arg("context"),
		nb::arg("numerator_out").none() = nb::none(),
		nb::arg("denominator_out").none() = nb::none()
	);
}

} // namespace em
} // namespace rexlib
