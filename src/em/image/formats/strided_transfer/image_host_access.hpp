// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{

class array_ref;
class const_array_ref;

namespace em
{

/**
 * @brief Get where an array holds its values, on the host.
 *
 * @param array The array to reach.
 * @return void* Its first byte of storage, never null.
 * @throws std::invalid_argument If @p array is not initialized.
 * @throws unsupported_capability_error If its storage is not host accessible.
 */
void* get_host_data(array_ref array);

/**
 * @brief Get where an array holds its values, on the host, to read them.
 *
 * @param array The array to reach.
 * @return const void* Its first byte of storage, never null.
 * @throws std::invalid_argument If @p array is not initialized.
 * @throws unsupported_capability_error If its storage is not host accessible.
 */
const void* get_host_data(const_array_ref array);

} // namespace em
} // namespace rexlib
