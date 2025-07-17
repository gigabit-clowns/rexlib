// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/compute/device_copy.hpp>

namespace xmipp4 
{
namespace compute
{

class cuda_device_buffer;
class cuda_device_queue;

/**
 * @brief CUDA implementation of the buffer copy engine.
 * 
 */
class cuda_device_copy final
    : public device_copy
{
public:
    void copy(const device_buffer &src_buffer,
              device_buffer &dst_buffer, 
              device_queue &queue ) override;

    void copy(const cuda_device_buffer &src_buffer,
              cuda_device_buffer &dst_buffer, 
              cuda_device_queue &queue );

    void copy(const device_buffer &src_buffer,
              device_buffer &dst_buffer,
              span<const copy_region> regions,
              device_queue &queue ) override;

    void copy(const cuda_device_buffer &src_buffer,
              cuda_device_buffer &dst_buffer,
              span<const copy_region> regions,
              cuda_device_queue &queue );

}; 

} // namespace compute
} // namespace xmipp4
