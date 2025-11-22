// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/buffer.hpp>

#include "../dynamic_shared_object.h"

namespace xmipp4 
{
namespace hardware
{

class cuda_buffer final
	: public buffer
{
public:
	XMIPP4_HARDWARE_CUDA_API
	cuda_buffer(
		void *device_pointer,
		void *host_pointer,
		std::size_t size,
		std::reference_wrapper<memory_resource> resource,
		std::unique_ptr<buffer_sentinel> sentinel
	);
	~cuda_buffer() override = default;

	/**
	 * @brief Get the CUDA device accessible pointer.
	 * 
	 * @return void* Pointer to the data.
	 */
	XMIPP4_HARDWARE_CUDA_API
	void* get_device_ptr() noexcept;

	/**
	 * @brief Get the CUDA device accessible pointer.
	 * 
	 * @return void* Pointer to the data.
	 */
	XMIPP4_HARDWARE_CUDA_API
	const void* get_device_ptr() const noexcept;

private:
	void *m_device_ptr;

}; 

/**
 * @brief Get a CUDA device accessible pointer from a generic buffer.
 * 
 * @param buf The buffer from which to get the device pointer.
 * @return const void* The device accessible pointer. nullptr if the buffer
 * is not a cuda_buffer or it is not device accessible.
 */
void* cuda_get_device_ptr(buffer& buf) noexcept;

/**
 * @brief Get a CUDA device accessible pointer from a generic buffer.
 * 
 * @param buf The buffer from which to get the device pointer.
 * @return const void* The device accessible pointer. nullptr if the buffer
 * is not a cuda_buffer or it is not device accessible.
 */
const void* cuda_get_device_ptr(const buffer& buf) noexcept;

} // namespace hardware
} // namespace xmipp4
