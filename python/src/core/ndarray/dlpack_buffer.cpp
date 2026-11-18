// SPDX-License-Identifier: GPL-3.0-only

#include "dlpack_buffer.hpp"

#include <rexlib/core/hardware/memory_resource.hpp>

namespace rexlib
{

namespace
{

template <typename Managed>
void release(void *tensor)
{
	auto *managed = static_cast<Managed*>(tensor);
	if (managed->deleter != nullptr)
	{
		managed->deleter(managed);
	}
}

} // anonymous namespace

dlpack_buffer::dlpack_buffer(
	DLManagedTensor *tensor,
	void *data,
	std::size_t size
) noexcept
	: m_tensor(tensor)
	, m_release(&release<DLManagedTensor>)
	, m_data(data)
	, m_size(size)
{
}

dlpack_buffer::dlpack_buffer(
	DLManagedTensorVersioned *tensor,
	void *data,
	std::size_t size
) noexcept
	: m_tensor(tensor)
	, m_release(&release<DLManagedTensorVersioned>)
	, m_data(data)
	, m_size(size)
{
}

dlpack_buffer::~dlpack_buffer()
{
	m_release(m_tensor);
}

void* dlpack_buffer::get_host_ptr() noexcept
{
	return m_data;
}

const void* dlpack_buffer::get_host_ptr() const noexcept
{
	return m_data;
}

std::size_t dlpack_buffer::get_size() const noexcept
{
	return m_size;
}

const memory_resource& dlpack_buffer::get_memory_resource() const noexcept
{
	return get_host_memory_resource();
}

} // namespace rexlib
