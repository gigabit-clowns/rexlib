// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "memory_resource_affinity.hpp"

#include <rex/core/platform/dynamic_shared_object.h>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Describes a scratch buffer requirement for program execution.
 *
 * A program may require scratch (temporary) memory buffers during execution.
 * This class encapsulates a single scratch buffer requirement, specifying:
 * - The required memory size in bytes.
 * - The required memory alignment in bytes. Must be a power of two.
 * - The memory affinity (where the memory should be allocated)
 *
 * This is an immutable value object. Once constructed, its properties cannot
 * be changed. Instances are typically lightweight and efficiently copied or
 * moved.
 */
class REXLIB_API program_scratch_requirement
{
public:
	/**
	 * @brief Constructs a scratch buffer requirement.
	 *
	 * @param size Size in bytes of the required scratch buffer.
	 * @param alignment Required alignment in bytes for the scratch buffer.
	 * Must be a power of two.
	 * @param affinity Where the memory should be allocated.
	 *
	 * @note The constructor does not validate that alignment is a power of two;
	 * callers are responsible for providing valid values.
	 */
	program_scratch_requirement(
		std::size_t size,
		std::size_t alignment,
		memory_resource_affinity affinity
	) noexcept;

	program_scratch_requirement(
		const program_scratch_requirement &other
	) = default;
	program_scratch_requirement(program_scratch_requirement &&other) = default;
	~program_scratch_requirement() = default;

	program_scratch_requirement&
	operator=(const program_scratch_requirement &other) = default;
	program_scratch_requirement&
	operator=(program_scratch_requirement &&other) = default;

	/**
	 * @brief Returns the required scratch buffer size in bytes.
	 *
	 * @return Size in bytes.
	 */
	std::size_t get_size() const noexcept;

	/**
	 * @brief Returns the required alignment in bytes for the scratch buffer.
	 *
	 * @return Alignment in bytes (a power of two).
	 */
	std::size_t get_alignment() const noexcept;

	/**
	 * @brief Returns the memory affinity for the scratch buffer.
	 *
	 * @return memory_resource_affinity indicating whether the buffer should
	 * be allocated for host or device access.
	 */
	memory_resource_affinity get_affinity() const noexcept;

private:
	std::size_t m_size;
	std::size_t m_alignment;
	memory_resource_affinity m_affinity;
};

} // namespace rexlib
