// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/em_component.hpp>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>
#include <rexlib/em/ops/reconstruction/slice_insertion_rules.hpp>

namespace rexlib
{
namespace em
{

REXLIB_DECLARE_OPERATION_TRAITS(
	insert_ctf_slices,
	em_component,
	REXLIB_OPERANDS("numerator", "denominator"),
	REXLIB_OPERANDS(
		"numerator",
		"denominator",
		"slice",
		"transform",
		"ctf",
		"radial_weight"
	),
	ctf_slice_insertion_shape_rule,
	ctf_slice_insertion_type_rule
);

/**
 * @brief Insert the spectra of projections into the spectrum of a volume,
 * weighted by their contrast transfer function.
 *
 * Works as insert_slices_operation, with one more input after `transform`:
 * `ctf`, the four CTF parameters of each slice, with the core axis (4):
 * 1. The defocus D_u along the direction of the angle a, in angstroms.
 * 2. The defocus D_v across that direction, in angstroms.
 * 3. The angle a, from the x axis towards the y axis, in radians.
 * 4. The phase shift f, in radians.
 *
 * A positive defocus is an underfocus. With c(k) the CTF of the frequency
 * k of a slice:
 * - numerator[g] += kappa(g - K) r c(k) v exp(-2 pi i g . C / N)
 * - denominator[g] += kappa(g - K) r c(k)^2
 *
 * The CTF is
 * c(k) = sin(pi L D s^2 - (pi / 2) Cs L^3 s^4 + f + asin(Q)), where:
 * - L is the wavelength of the optics, in angstroms;
 * - Cs is the spherical aberration of the optics, converted to angstroms
 *   (1 mm is 1e7 angstroms);
 * - Q is the amplitude contrast of the optics;
 * - s = |k| / (n p) is the spatial frequency in inverse angstroms, with p
 *   the pixel size;
 * - D = (D_u + D_v) / 2 + (D_u - D_v) / 2 cos(2 (t - a)) is the defocus
 *   along the direction t = atan2(k_y, k_x) of k.
 *
 * So c(0) = Q, and the CTF is positive at low frequencies. The frequency
 * -k that an element also stands for has the same CTF.
 *
 * @see insert_slices_operation
 * @see ctf_optics
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API insert_ctf_slices_operation final
	: public trivial_operation<
		insert_ctf_slices_operation,
		insert_ctf_slices_operation_traits
	>
{
public:
	/**
	 * @brief Construct an insertion weighted by the CTF.
	 *
	 * @param window The window that spreads a sample over the voxels.
	 * @param optics The optics of the microscope.
	 * @param pixel_size The size of a pixel of the images, in angstroms.
	 *
	 * @throws std::invalid_argument When the pixel size is not finite and
	 * positive.
	 */
	insert_ctf_slices_operation(
		const gridding_window &window,
		const ctf_optics &optics,
		double pixel_size
	);

	/**
	 * @brief Get the gridding window.
	 *
	 * @return const gridding_window& The window.
	 */
	const gridding_window& get_window() const noexcept;

	/**
	 * @brief Get the optics of the microscope.
	 *
	 * @return const ctf_optics& The optics.
	 */
	const ctf_optics& get_optics() const noexcept;

	/**
	 * @brief Get the size of a pixel of the images.
	 *
	 * @return double The size, in angstroms.
	 */
	double get_pixel_size() const noexcept;

private:
	gridding_window m_window;
	ctf_optics m_optics;
	double m_pixel_size;
};
REXLIB_END_TEMPLATE_BASE

} // namespace em
} // namespace rexlib
