#pragma once

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_buffer_memcpy.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines functions to copy between cuda buffers
 * @date 2024-12-07
 * 
 */

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

