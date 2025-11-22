// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/hardware/cuda_buffer.hpp>

namespace xmipp4
{
namespace hardware
{

cuda_buffer::cuda_buffer(
	void *device_pointer,
	void *host_pointer,
	std::size_t size,
	std::reference_wrapper<memory_resource> resource,
	std::unique_ptr<buffer_sentinel> sentinel
)
	: buffer(host_pointer, size, resource, std::move(sentinel))
	, m_device_ptr(device_pointer)
{
}

void* cuda_buffer::get_device_ptr() noexcept
{
	return m_device_ptr;
}

const void* cuda_buffer::get_device_ptr() const noexcept
{
	return m_device_ptr;
}



void* cuda_get_device_ptr(buffer& buf) noexcept
{
	auto *cuda_buf = dynamic_cast<cuda_buffer*>(&buf);
	if (!cuda_buf)
	{
		return nullptr;
	}

	return cuda_buf->get_device_ptr();
}


const void* cuda_get_device_ptr(const buffer& buf) noexcept
{
	const auto *cuda_buf = dynamic_cast<const cuda_buffer*>(&buf);
	if (!cuda_buf)
	{
		return nullptr;
	}

	return cuda_buf->get_device_ptr();
}

} // namespace hardware
} // namespace xmipp4
