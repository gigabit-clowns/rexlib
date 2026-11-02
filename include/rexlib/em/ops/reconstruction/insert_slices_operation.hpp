// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/operation/operation_declaration_macros.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/em_component.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>
#include <rexlib/em/ops/reconstruction/slice_insertion_rules.hpp>

namespace rexlib
{
namespace em
{

REXLIB_DECLARE_OPERATION_TRAITS(
	insert_slices,
	em_component,
	REXLIB_OPERANDS("numerator", "denominator"),
	REXLIB_OPERANDS(
		"numerator",
		"denominator",
		"slice",
		"transform",
		"radial_weight"
	),
	slice_insertion_shape_rule,
	slice_insertion_type_rule
);

/**
 * @brief Insert the spectra of projections into the spectrum of a volume.
 *
 * Each slice is the half spectrum of a projection of a volume. By the
 * central slice theorem, it is a central plane of the spectrum of the
 * volume. The operation adds the slices to a numerator and their weights to
 * a denominator. The numerator divided by the denominator estimates the
 * half spectrum of the volume.
 *
 * @par Operands
 * - `numerator` and `denominator`: the accumulators, complex and real. They
 *   are half spectra of a cubic volume, with the core axes
 *   (N, N, N / 2 + 1).
 * - `slice`: half spectra of square images, with the core axes
 *   (n, n / 2 + 1), as rfft2 returns them.
 * - `transform`: 3x4 matrices [A | t], one per slice.
 * - `radial_weight`: a weight per frequency shell, with the core axis
 *   (n / 2 + 1).
 *
 * The outputs are the accumulators plus the insertions. Passing the
 * accumulators as the outputs updates them in place. Any other overlap
 * between an output and an input gives unspecified values.
 *
 * @par Batches
 * The batch axes of all inputs broadcast together. The broadcast batch axes
 * that an accumulator lacks, or holds with an extent of 1, are summed. For
 * example, accumulators of batch (K, 1) and transforms of batch (K, B)
 * insert B slices into each of K volumes.
 *
 * @par Coordinates
 * The x axis is the last axis, y the one before it and z the one before
 * that. The origin of an image or a volume of extent m lies at the index
 * m / 2, rounded down, along each axis.
 *
 * @par Geometry
 * The transform maps the point p = (x, y, z) of the volume, in voxels from
 * its origin, to the point A p + t. Its first two coordinates are the
 * (x, y) of the image, in pixels from its origin. The image is the
 * projection of the volume along the third coordinate. Only the first two
 * rows of the transform are read, and A need not be orthonormal.
 *
 * @par Frequencies
 * A frequency is a signed index: numpy.fft.fftfreq times the extent along
 * the y and z axes, and numpy.fft.rfftfreq times the extent along the x
 * axis. The frequency k = (k_x, k_y) of a slice lands on the frequency
 * K = (N / n) transpose(A2) k of the volume, where A2 holds the first two
 * rows of A. An N larger than n therefore pads the volume.
 *
 * @par Phases
 * The element X(k) of a slice enters as
 * v = X(k) exp(2 pi i k . (c + t2) / n), where c = (n / 2, n / 2) rounded
 * down and t2 holds the first two elements of t. A contribution to the
 * voxel g is multiplied by exp(-2 pi i g . C / N), where C = (N / 2, N / 2,
 * N / 2) rounded down. So the slices are the plain spectra of the images,
 * and the inverse transform of the quotient is a volume with its origin
 * at C.
 *
 * @par Inserted frequencies
 * Every frequency of the full spectrum of a slice is inserted once. An
 * element with k_x > 0 also stands for -k, which carries the conjugate of
 * v. A frequency is not inserted when:
 * - its shell, |k| rounded to the nearest integer with halves up, exceeds
 *   n / 2 rounded down;
 * - n is even and its x or y frequency is n / 2 or -n / 2;
 * - the radial weight of its shell is 0.
 *
 * @par Accumulation
 * The value at K is spread over the voxels g with the weight kappa(g - K) of
 * the gridding window. With r the radial weight of the shell of k:
 * - numerator[g] += kappa(g - K) r v exp(-2 pi i g . C / N)
 * - denominator[g] += kappa(g - K) r
 *
 * The sums run over the full spectrum of each slice and are defined at
 * every voxel g of the full spectrum of the volume. The accumulators hold
 * the sums at the voxels with g_x >= 0; those at g_x < 0 are their
 * conjugates, and are not stored. A voxel beyond N / 2 in magnitude along
 * any axis receives nothing. Along y and z of an even N, the frequencies
 * N / 2 and -N / 2 are the same element. The order of the sums is
 * unspecified.
 *
 * @see gridding_window
 */
REXLIB_BEGIN_TEMPLATE_BASE
class REXLIB_API insert_slices_operation final
	: public trivial_operation<
		insert_slices_operation,
		insert_slices_operation_traits
	>
{
public:
	/**
	 * @brief Construct an insertion with a gridding window.
	 *
	 * @param window The window that spreads a sample over the voxels.
	 */
	explicit insert_slices_operation(const gridding_window &window) noexcept;

	/**
	 * @brief Get the gridding window.
	 *
	 * @return const gridding_window& The window.
	 */
	const gridding_window& get_window() const noexcept;

private:
	gridding_window m_window;
};
REXLIB_END_TEMPLATE_BASE

} // namespace em
} // namespace rexlib
