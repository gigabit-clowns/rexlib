// SPDX-License-Identifier: GPL-3.0-only

#include "image_scratch_storage_buffer.hpp"

#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/platform/assert.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

#include <utility>

namespace rexlib
{
namespace em
{

image_scratch_storage_buffer::image_scratch_storage_buffer(
	std::shared_ptr<image_scratch_storage> storage
) noexcept
	: m_storage(std::move(storage))
{
	REXLIB_ASSERT(m_storage);
}

image_scratch_storage_buffer::~image_scratch_storage_buffer() = default;

void* image_scratch_storage_buffer::get_host_ptr() noexcept
{
	return m_storage->get_data();
}

const void* image_scratch_storage_buffer::get_host_ptr() const noexcept
{
	const image_scratch_storage &storage = *m_storage;
	return storage.get_data();
}

std::size_t image_scratch_storage_buffer::get_size() const noexcept
{
	return m_storage->get_size();
}

const memory_resource&
image_scratch_storage_buffer::get_memory_resource() const noexcept
{
	return get_host_memory_resource();
}

} // namespace em
} // namespace rexlib
