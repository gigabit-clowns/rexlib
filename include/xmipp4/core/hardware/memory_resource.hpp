// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>

#include <xmipp4/core/platform/dynamic_shared_object.h>

#include "memory_resource_kind.hpp"

namespace xmipp4
{

class memory_allocator;
class device;

/**
 * @brief Abstract description of a physical memory region.
 */
class REXLIB_API memory_resource
{
public:
	memory_resource() noexcept;
	memory_resource(const memory_resource &other) = delete;
	memory_resource(memory_resource &&other) = delete;
	virtual ~memory_resource();

	memory_resource& operator=(const memory_resource &other) = delete;
	memory_resource& operator=(memory_resource &&other) = delete;

	/**
	 * @brief Get the kind of memory this resource represents.
	 *
	 * The returned value is constant over the lifetime of the resource and 
	 * determines the host/device accessibility of the buffers allocated from it
	 * (see @ref is_host_accessible and @ref is_device_accessible).
	 *
	 * @return Kind tag describing this resource.
	 */
	virtual
	memory_resource_kind get_kind() const noexcept = 0;

	/**
	 * @brief Create an allocator that produces buffers on this resource.
	 *
	 * Some implementations may choose to always return the same allocator
	 * object (singleton-like).
	 *
	 * @return The new allocator. Never null.
	 */
	virtual
	std::shared_ptr<memory_allocator> create_allocator() const = 0;
};

/**
 * @brief Get the resource representing host memory.
 *
 * The returned resource has kind @ref memory_resource_kind::host. It represents
 * plain host-addressable memory.
 *
 * @return Reference to the host memory resource singleton. The reference is 
 * valid for the entire lifetime of the process.
 */
REXLIB_API
const memory_resource& get_host_memory_resource() noexcept;

} // namespace xmipp4
