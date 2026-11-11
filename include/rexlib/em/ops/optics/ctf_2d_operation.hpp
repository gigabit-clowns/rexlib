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
	ctf_2d,
	em_component,
	REXLIB_OPERANDS("ctf"),
	REXLIB_OPERANDS("parameters"),
	ctf_2d_shape_rule,
	ops::signal_extent,
	ctf_type_rule
);

/**
 * @brief Evaluate the contrast transfer function over the half spectrum of
 * square images.
 *
 * `parameters` holds the four CTF parameters of each image, with the core
 * axis (4), as insert_ctf_slices_operation reads them:
 * 1. The defocus D_u along the direction of the angle a, in angstroms.
 * 2. The defocus D_v across that direction, in angstroms.
 * 3. The angle a, from the x axis towards the y axis, in radians.
 * 4. The phase shift f, in radians.
 *
 * `ctf` has the core axes (n, n / 2 + 1), the half spectrum of an image of
 * n by n pixels as rfft lays it out, and the batch axes of `parameters`.
 * The element (i, j) holds c(k) at the frequency k = (j, k_y), with
 * k_y = i for i < (n + 1) / 2 and i - n otherwise. The CTF c is that of
 * insert_ctf_slices_operation.
 *
 * @see insert_ctf_slices_operation
 * @see ctf_1d_operation
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API ctf_2d_operation final
	: public parametric_operation<ctf_2d_operation, ctf_2d_operation_traits>
{
public:
	/**
	 * @brief Construct the evaluation of the CTF of square images.
	 *
	 * @param extent The number of pixels n along each axis of an image.
	 * @param optics The optics of the microscope.
	 * @param pixel_size The size of a pixel, in angstroms.
	 *
	 * @throws std::invalid_argument When @p extent is zero, or the pixel
	 * size is not finite and positive.
	 */
	ctf_2d_operation(
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
	 * @brief Get the size of a pixel of the images.
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
