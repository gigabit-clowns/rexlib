// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/memory_resource.hpp>

namespace rex 
{

class host_memory_resource final
	: public memory_resource
{
public:
	~host_memory_resource() override;

	memory_resource_kind get_kind() const noexcept override;

	std::shared_ptr<memory_allocator> create_allocator() const override;

	static const host_memory_resource& get() noexcept;

private:
	host_memory_resource() noexcept;
	host_memory_resource(const host_memory_resource &other) = delete; // Singleton
	host_memory_resource(host_memory_resource &&other) = delete; // Singleton

	static const host_memory_resource m_instance;
};

} // namespace rex
