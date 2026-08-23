// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/caching_allocator/queue_handle.hpp>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief Make a handle standing for a queue, without creating one.
 *
 * A pool only ever compares stream handles and passes them back to the
 * recorder, so a test can name its queues without a device to create them on.
 *
 * @param index Distinguishes one queue from another. Handles made with the
 * same index compare equal, and different indices never collide.
 * @param ordinal Ordinal of the device the queue would belong to.
 * @return queue_handle The handle.
 */
queue_handle make_test_queue(unsigned index, int ordinal = 0) noexcept;

} // namespace cuda
} // namespace xmipp4
