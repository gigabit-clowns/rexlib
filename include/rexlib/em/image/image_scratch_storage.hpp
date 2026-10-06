// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/memory/byte.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{
namespace em
{

/**
 * @brief Memory in which a scratch keeps its copies.
 *
 * A block of bytes that the host can read and write. Where the block lives
 * is up to the implementation: in main memory, or in a file.
 *
 * @par Thread safety
 * Every method may be called concurrently. Threads may read and write
 * different bytes of the block at the same time.
 */
class REXLIB_API image_scratch_storage
{
public:
	image_scratch_storage() noexcept;
	image_scratch_storage(const image_scratch_storage &other) = delete;
	image_scratch_storage(image_scratch_storage &&other) = delete;
	virtual ~image_scratch_storage();

	image_scratch_storage&
	operator=(const image_scratch_storage &other) = delete;
	image_scratch_storage& operator=(image_scratch_storage &&other) = delete;

	/**
	 * @brief Get the block of bytes.
	 *
	 * @return byte* The first byte. Never null. It is valid for as long as
	 * this storage is alive.
	 */
	virtual byte* get_data() noexcept = 0;

	/**
	 * @brief Get the block of bytes, to read them.
	 *
	 * @return const byte* The first byte. Never null. It is valid for as
	 * long as this storage is alive.
	 */
	virtual const byte* get_data() const noexcept = 0;

	/**
	 * @brief Get the size of the block.
	 *
	 * @return std::size_t The number of bytes, which does not change.
	 */
	virtual std::size_t get_size() const noexcept = 0;
};

} // namespace em
} // namespace rexlib
