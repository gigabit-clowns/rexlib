// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/hardware/memory_resource.hpp>

#include <memory>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief Page locked host memory, shared by every CUDA device in the process.
 *
 * Page locking is what lets a transfer run asynchronously and at full speed,
 * and what is locked is host memory: it belongs to the process rather than to
 * any one device, and a page locked region taken while one device was current
 * serves all of them. One of these therefore exists per process, not per
 * device, which also keeps several devices from each holding an arena of a
 * resource the whole machine shares.
 *
 * Where the host and the device are the same physical memory, this is not
 * staging for a transfer but the memory itself, and it says so.
 *
 * Each allocator asked for is one of its own, holding a cache of its own.
 * Sharing a cache means sharing the allocator, which is what a
 * @ref xmipp4::device_context is for; asking for another one here is asking
 * for another cache.
 */
class pinned_memory_resource final
	: public memory_resource
{
public:
	~pinned_memory_resource() override;

	memory_resource_kind get_kind() const noexcept override;

	std::shared_ptr<memory_allocator> create_allocator() const override;

	/**
	 * @brief Get the page locked host memory of the process.
	 *
	 * Built the first time it is asked for, since working out what kind of
	 * memory it is means asking the driver what devices there are.
	 *
	 * @return const pinned_memory_resource& The resource. Valid for the rest
	 * of the process.
	 */
	static const pinned_memory_resource& get();

private:
	pinned_memory_resource();
	pinned_memory_resource(const pinned_memory_resource &other) = delete;
	pinned_memory_resource(pinned_memory_resource &&other) = delete;

	pinned_memory_resource&
	operator=(const pinned_memory_resource &other) = delete;
	pinned_memory_resource&
	operator=(pinned_memory_resource &&other) = delete;

	memory_resource_kind m_kind;
};

} // namespace cuda
} // namespace xmipp4
