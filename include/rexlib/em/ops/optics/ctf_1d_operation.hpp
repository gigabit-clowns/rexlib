// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/em_component.hpp>
#include <rexlib/em/ops/optics/ctf_rules.hpp>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>
#include <rexlib/ops/parameters/signal_extent.hpp>

#include <cstddef>

namespace rexlib
{
namespace em
{

REXLIB_DECLARE_PARAMETRIC_OPERATION_TRAITS(
	ctf_1d,
	em_component,
	REXLIB_OPERANDS("ctf"),
	REXLIB_OPERANDS("parameters"),
	ctf_1d_shape_rule,
	ops::signal_extent,
	ctf_type_rule
);

/**
 * @brief Evaluate the contrast transfer function over the half spectrum of
 * 1D signals.
 *
 * `parameters` holds the two CTF parameters of each signal, with the core
 * axis (2):
 * 1. The defocus D, in angstroms.
 * 2. The phase shift f, in radians.
 *
 * `ctf` has the core axis (n / 2 + 1), the half spectrum of a signal of n
 * samples as rfft lays it out, and the batch axes of `parameters`. The
 * element k holds c(k) = sin(pi L D s^2 - (pi / 2) Cs L^3 s^4 + f +
 * asin(Q)), with s = k / (n p) and L, Cs, Q and p as in ctf_2d_operation.
 * A 1D signal has no direction, so the CTF has no astigmatism: it is that
 * of ctf_2d_operation with D_u = D_v = D along the x axis.
 *
 * @see ctf_2d_operation
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API ctf_1d_operation final
	: public parametric_operation<ctf_1d_operation, ctf_1d_operation_traits>
{
public:
	/**
	 * @brief Construct the evaluation of the CTF of 1D signals.
	 *
	 * @param extent The number of samples n of a signal.
	 * @param optics The optics of the microscope.
	 * @param pixel_size The size of a sample, in angstroms.
	 *
	 * @throws std::invalid_argument When @p extent is zero, or the pixel
	 * size is not finite and positive.
	 */
	ctf_1d_operation(
		std::size_t extent,
		const ctf_optics &optics,
		double pixel_size
	);

	/**
	 * @brief Get the optics of the microscope.
	 *
	 * @return const ctf_optics& The optics.
	 */
	const ctf_optics& get_optics() const noexcept;

	/**
	 * @brief Get the size of a sample of the signals.
	 *
	 * @return double The size, in angstroms.
	 */
	double get_pixel_size() const noexcept;

private:
	ctf_optics m_optics;
	double m_pixel_size;
};
REXLIB_END_TEMPLATE_BASE

} // namespace em
} // namespace rexlib
