// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_allocator.hpp>
#include <xmipp4/core/hardware/memory_resource_affinity.hpp>

#include <memory>
#include <array>

namespace rex
{

class device;
class memory_resource;

/**
 * @brief Owns one memory_allocator per memory_resource_affinity value.
 *
 * Aggregates the allocators required to serve allocation requests on a
 * single device. Each memory_resource_affinity slot holds a
 * shared_ptr<memory_allocator> that can be queried or replaced individually.
 *
 * The table is move-only: copy construction and copy assignment are deleted.
 */
class memory_allocator_table
{
public:
	/**
	 * @brief Construct an empty table with no allocators installed.
	 *
	 * All affinity slots are initialized to nullptr.
	 */
	memory_allocator_table() = default;

	/**
	 * @brief Construct by querying @p dev for its memory resources and
	 * selecting the best allocator for each affinity.
	 *
	 * Depending on the memory architecture of the device, resulting allocators
	 * may alias each other.
	 * 
	 * @param dev Device for which allocators are instantiated.
	 *
	 * @throws std::runtime_error if a required memory_resource_affinity
	 * has no suitable resource on @p dev.
	 */
	explicit memory_allocator_table(const device &dev);
	memory_allocator_table(
		const memory_allocator_table &other) = delete;
	memory_allocator_table(
		memory_allocator_table &&other) = default;
	~memory_allocator_table() = default;

	memory_allocator_table&
	operator=(const memory_allocator_table &other) = delete;
	memory_allocator_table&
	operator=(memory_allocator_table &&other) = default;

	/**
	 * @brief Replace the allocator for the given memory affinity.
	 *
	 * @param affinity The memory_resource_affinity slot to update.
	 * @param allocator New allocator to install; may be nullptr to clear
	 * the slot.
	 * @return The allocator that was previously installed in that slot,
	 * or nullptr if the slot was empty.
	 */
	std::shared_ptr<memory_allocator>
	set_allocator(
		memory_resource_affinity affinity,
		std::shared_ptr<memory_allocator> allocator
	) noexcept;

	/**
	 * @brief Retrieve the allocator for the given memory affinity.
	 *
	 * @param affinity The desired memory_resource_affinity (host or device).
	 * @return Shared pointer to the corresponding memory_allocator.
	 */
	const std::shared_ptr<memory_allocator>&
	get_allocator(memory_resource_affinity affinity) const noexcept;

private:
	using allocator_array_type = std::array<
		std::shared_ptr<memory_allocator>,
		static_cast<std::size_t>(memory_resource_affinity::count)
	>;
	allocator_array_type m_allocators;
};

} // namespace rex
