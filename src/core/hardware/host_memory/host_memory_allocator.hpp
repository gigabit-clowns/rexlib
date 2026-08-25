// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/memory_allocator.hpp>

namespace rexlib 
{

/**
 * @brief Implementation of memory_allocator specialized for allocating memory
 * in host.
 */
class host_memory_allocator final
	: public memory_allocator
{
public:
	host_memory_allocator() = default;
	~host_memory_allocator() override = default;

	const memory_resource& get_memory_resource() const noexcept override;

	std::size_t get_max_alignment() const noexcept override;

	std::shared_ptr<buffer> allocate(
		std::size_t size, 
		std::size_t alignment, 
		command_queue *queue
	) override;

	static host_memory_allocator& get();
	static const std::shared_ptr<host_memory_allocator>& create();

private:
	static std::shared_ptr<host_memory_allocator> m_instance;
};

} // namespace rexlib
