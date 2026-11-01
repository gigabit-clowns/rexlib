// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <utility>

namespace rexlib
{

class execution_context;

namespace em
{

/**
 * @brief Insert the spectra of projections into the spectrum of a volume.
 *
 * Pass the accumulators as @p numerator_out and @p denominator_out to
 * update them in place.
 *
 * @param numerator The complex accumulator of the slices, with the core
 * axes (N, N, N / 2 + 1).
 * @param denominator The real accumulator of the weights, with the core
 * axes (N, N, N / 2 + 1).
 * @param slices The half spectra of the projections, with the core axes
 * (n, n / 2 + 1).
 * @param transforms The 3x4 transforms from the volume to each projection.
 * @param radial_weights The weight of each frequency shell, with the core
 * axis (n / 2 + 1).
 * @param window The window that spreads a sample over the voxels.
 * @param context The execution context used for dispatching.
 * @param numerator_out Optional output parameter to be re-used for the
 * numerator.
 * @param denominator_out Optional output parameter to be re-used for the
 * denominator.
 * @return std::pair<array, array> The numerator and the denominator.
 *
 * @see insert_slices_operation
 */
REXLIB_API
std::pair<array, array> insert_slices(
	const_array_ref numerator,
	const_array_ref denominator,
	const_array_ref slices,
	const_array_ref transforms,
	const_array_ref radial_weights,
	const gridding_window &window,
	const execution_context &context,
	array *numerator_out = nullptr,
	array *denominator_out = nullptr
);

/**
 * @brief Insert the spectra of projections into the spectrum of a volume,
 * with a denominator weight per frequency.
 *
 * Pass the accumulators as @p numerator_out and @p denominator_out to
 * update them in place.
 *
 * @param numerator The complex accumulator of the slices, with the core
 * axes (N, N, N / 2 + 1).
 * @param denominator The real accumulator of the weights, with the core
 * axes (N, N, N / 2 + 1).
 * @param slices The half spectra of the projections, with the core axes
 * (n, n / 2 + 1).
 * @param weights The real denominator weight of each frequency of the
 * slices, with the core axes (n, n / 2 + 1).
 * @param transforms The 3x4 transforms from the volume to each projection.
 * @param radial_weights The weight of each frequency shell, with the core
 * axis (n / 2 + 1).
 * @param window The window that spreads a sample over the voxels.
 * @param context The execution context used for dispatching.
 * @param numerator_out Optional output parameter to be re-used for the
 * numerator.
 * @param denominator_out Optional output parameter to be re-used for the
 * denominator.
 * @return std::pair<array, array> The numerator and the denominator.
 *
 * @see insert_weighted_slices_operation
 */
REXLIB_API
std::pair<array, array> insert_weighted_slices(
	const_array_ref numerator,
	const_array_ref denominator,
	const_array_ref slices,
	const_array_ref weights,
	const_array_ref transforms,
	const_array_ref radial_weights,
	const gridding_window &window,
	const execution_context &context,
	array *numerator_out = nullptr,
	array *denominator_out = nullptr
);

} // namespace em
} // namespace rexlib
