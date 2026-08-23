// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_resource.hpp>

#include <memory>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief The memory of one CUDA device.
 *
 * Reachable only by the device it belongs to; the host has to transfer to and
 * from it.
 *
 * Each allocator asked for is one of its own, holding a cache of its own.
 * Sharing a cache means sharing the allocator, which is what a
 * @ref xmipp4::device_context is for; asking for another one here is asking
 * for another cache.
 */
class device_memory_resource final
	: public memory_resource
{
public:
	/**
	 * @brief Construct the resource of a device.
	 *
	 * @param ordinal Ordinal of the device the memory belongs to.
	 */
	explicit device_memory_resource(int ordinal);
	~device_memory_resource() override;

	/**
	 * @brief Get the device this memory belongs to.
	 *
	 * @return int The device ordinal.
	 */
	int get_ordinal() const noexcept;

	memory_resource_kind get_kind() const noexcept override;

	std::shared_ptr<memory_allocator> create_allocator() const override;

private:
	int m_ordinal;
};

} // namespace cuda
} // namespace xmipp4
