// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/core/ndarray/const_array_ref.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <cstddef>

namespace rexlib
{

class execution_context;

namespace em
{

/**
 * @brief Evaluate the contrast transfer function over the half spectrum of
 * 1D signals.
 *
 * @param parameters The defocus and the phase shift of each signal, with the
 * core axis (2).
 * @param extent The number of samples n of a signal.
 * @param optics The optics of the microscope.
 * @param pixel_size The size of a sample, in angstroms.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The CTF, a half spectrum after the batch axes of
 * @p parameters.
 *
 * @throws std::invalid_argument When @p extent is zero, or the pixel size
 * is not finite and positive.
 *
 * @see ctf_1d_operation
 */
REXLIB_API
array ctf_1d(
	const_array_ref parameters,
	std::size_t extent,
	const ctf_optics &optics,
	double pixel_size,
	const execution_context &context,
	array *out = nullptr
);

/**
 * @brief Evaluate the contrast transfer function over the half spectrum of
 * square images.
 *
 * @param parameters The four CTF parameters of each image, with the core
 * axis (4), as insert_ctf_slices reads them.
 * @param extent The number of pixels n along each axis of an image.
 * @param optics The optics of the microscope.
 * @param pixel_size The size of a pixel, in angstroms.
 * @param context The execution context used for dispatching.
 * @param out Optional output parameter to be re-used.
 * @return array The CTF, a half spectrum after the batch axes of
 * @p parameters.
 *
 * @throws std::invalid_argument When @p extent is zero, or the pixel size
 * is not finite and positive.
 *
 * @see ctf_2d_operation
 */
REXLIB_API
array ctf_2d(
	const_array_ref parameters,
	std::size_t extent,
	const ctf_optics &optics,
	double pixel_size,
	const execution_context &context,
	array *out = nullptr
);

} // namespace em
} // namespace rexlib
