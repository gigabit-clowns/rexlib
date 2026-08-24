// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/buffer.hpp>

namespace rexlib 
{

/**
 * @brief Implementation of the buffer interface to represent data in host
 * memory.
 */
class host_buffer final
	: public buffer
{
public:
	host_buffer(std::size_t size, std::size_t alignment);
	~host_buffer() override;

	void* get_host_ptr() noexcept override;
	const void* get_host_ptr() const noexcept override;
	std::size_t get_size() const noexcept override;
	const memory_resource& get_memory_resource() const noexcept override;

private:
	void *m_data;
	std::size_t m_size;
};

} // namespace rexlib
