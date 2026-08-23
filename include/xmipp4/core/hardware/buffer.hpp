// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/platform/dynamic_shared_object.h>

#include <utility>
#include <memory>
#include <cstddef>

namespace rex 
{

class memory_resource;
class memory_allocator;

/**
 * @brief Represents an untyped memory allocation at a given memory
 * resource.
 *
 * A buffer is a handle to a contiguous block of memory. It carries no type 
 * information of its own.
 */
class REXLIB_API buffer
{
public:
	buffer() noexcept;
	buffer(const buffer &other) = delete;
	buffer(buffer &&other) noexcept = delete;
	virtual ~buffer();

	buffer& operator=(const buffer &other) = delete;
	buffer& operator=(buffer &&other) noexcept = delete;

	/**
	 * @brief Get a host accessible pointer to the data.
	 *
	 * This method only returns a non-null pointer if the data is
	 * directly addressable by the host, i.e. when the kind of the
	 * underlying @ref memory_resource is one of: @c device_mapped,
	 * @c host_staging, @c host, @c unified or @c managed. For pure
	 * device-only resources the buffer must be transferred to a
	 * host-accessible buffer first.
	 *
	 * The returned pointer is valid for as long as this @c buffer
	 * instance is alive and is aligned to the alignment requested at
	 * allocation time. Reads or writes through it may need to be
	 * preceded by appropriate synchronization with any in-flight
	 * device-side work that uses the same buffer.
	 *
	 * @return void* Pointer to the start of the buffer's data, or
	 * @c nullptr if the buffer is not host accessible.
	 */
	virtual void* get_host_ptr() noexcept = 0;

	/**
	 * @brief Get a host accessible pointer to the data.
	 *
	 * Const overload of @ref get_host_ptr(); see that method for the
	 * conditions under which a non-null pointer is returned and the
	 * lifetime/synchronization guarantees that apply.
	 *
	 * @return const void* Pointer to the start of the buffer's data, or
	 * @c nullptr if the buffer is not host accessible.
	 */
	virtual const void* get_host_ptr() const noexcept = 0;

	/**
	 * @brief Get the size in bytes for this buffer.
	 *
	 * The size may be larger then the requested in 
	 * @ref memory_allocator::allocate.
	 *
	 * @return std::size_t Size in bytes.
	 */
	virtual std::size_t get_size() const noexcept = 0;

	/**
	 * @brief Get the memory resource where this buffer is stored.
	 *
	 * @return const memory_resource& The resource backing the buffer.
	 */
	virtual const memory_resource& get_memory_resource() const noexcept = 0;
};

} // namespace rex
