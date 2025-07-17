// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/span.hpp>

namespace xmipp4 
{
namespace compute
{

class copy_region;
class host_buffer;
class cuda_device_buffer;
class cuda_device_queue;

/**
 * @brief Copy the whole buffer.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const cuda_device_buffer &src, 
                 cuda_device_buffer &dst,
                 cuda_device_queue &queue );

/**
 * @brief Copy regions of a buffer.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param regions Regions to be copied.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const cuda_device_buffer &src, 
                 cuda_device_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue );

/**
 * @brief Copy the whole buffer from device to host.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const cuda_device_buffer &src, 
                 host_buffer &dst,
                 cuda_device_queue &queue );

/**
 * @brief Copy regions of a buffer from device to host.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param regions Regions to be copied.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const cuda_device_buffer &src, 
                 host_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue );

/**
 * @brief Copy the whole buffer from host to device.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const host_buffer &src, 
                 cuda_device_buffer &dst,
                 cuda_device_queue &queue );

/**
 * @brief Copy regions of a buffer from host to device.
 * 
 * @param src Source buffer.
 * @param dst Destination buffer.
 * @param regions Regions to be copied.
 * @param queue Queue where the transfer is executed.
 */
void cuda_memcpy(const host_buffer &src, 
                 cuda_device_buffer &dst,
                 span<const copy_region> regions,
                 cuda_device_queue &queue );

} // namespace compute
} // namespace xmipp4
