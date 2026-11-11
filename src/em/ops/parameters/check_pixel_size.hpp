// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace em
{

/**
 * @brief Check the pixel size an operation is given.
 *
 * @param operation The name of the operation, which the message starts
 * with.
 * @param pixel_size The size of a pixel, in angstroms.
 * @return double The pixel size.
 *
 * @throws std::invalid_argument When the pixel size is not finite and
 * positive.
 */
double check_pixel_size(const char *operation, double pixel_size);

} // namespace em
} // namespace rexlib
