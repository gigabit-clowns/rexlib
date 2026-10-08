// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"

namespace rexlib
{

class array_ref;
class const_array_ref;

/**
 * @brief Get where an array holds its values, on the host, to write them.
 *
 * Blocks the calling thread until the commands that read or write the
 * memory of the array have finished.
 *
 * @param array The array to reach.
 * @return void* Its first byte of storage, never null.
 * @throws std::invalid_argument If @p array is not initialized.
 * @throws unsupported_capability_error If its storage is not host accessible.
 *
 * @see access_hazard_tracker
 */
REXLIB_API
void* get_host_data(array_ref array);

/**
 * @brief Get where an array holds its values, on the host, to read them.
 *
 * Blocks the calling thread until the commands that write the memory of the
 * array have finished.
 *
 * @param array The array to reach.
 * @return const void* Its first byte of storage, never null.
 * @throws std::invalid_argument If @p array is not initialized.
 * @throws unsupported_capability_error If its storage is not host accessible.
 *
 * @see access_hazard_tracker
 */
REXLIB_API
const void* get_host_data(const_array_ref array);

} // namespace rexlib
