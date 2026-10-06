// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/buffer.hpp>

#include <cstddef>
#include <memory>

namespace rexlib
{
namespace em
{

class image_scratch_storage;

/**
 * @brief Presents the storage of a scratch as a buffer.
 *
 * Arrays are built on buffers, so a scratch that keeps its copies as arrays
 * needs its storage as one. The buffer is host memory, and keeps the
 * storage alive.
 */
class image_scratch_storage_buffer final
	: public buffer
{
public:
	/**
	 * @brief Construct a buffer over a storage.
	 *
	 * @param storage The storage. Must not be null.
	 */
	explicit image_scratch_storage_buffer(
		std::shared_ptr<image_scratch_storage> storage
	) noexcept;

	~image_scratch_storage_buffer() override;

	void* get_host_ptr() noexcept override;
	const void* get_host_ptr() const noexcept override;
	std::size_t get_size() const noexcept override;
	const memory_resource& get_memory_resource() const noexcept override;

private:
	std::shared_ptr<image_scratch_storage> m_storage;
};

} // namespace em
} // namespace rexlib
