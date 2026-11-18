// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace cuda
{

/**
 * @brief Check whether the tests can reach a CUDA device.
 *
 * What only the driver can answer is tested on a device, and skipped where
 * there is none.
 *
 * @return true There is at least one device.
 * @return false There is none, or the driver could not be asked.
 */
bool has_test_device() noexcept;

} // namespace cuda
} // namespace rexlib
