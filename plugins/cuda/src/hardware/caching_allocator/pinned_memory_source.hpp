// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "memory_source.hpp"

namespace rexlib
{
namespace cuda
{

/**
 * @brief Page-locked host memory, as obtained from cudaHostAlloc.
 *
 * Page-locked memory is a process-wide resource rather than a per-device one:
 * it is portable across every device in the process, so a single source can
 * back all of them.
 */
class pinned_memory_source final
	: public memory_source
{
public:
	/**
	 * @brief Construct a source of page-locked host memory.
	 *
	 * @param mapped Whether the memory is also mapped into the device address
	 * space. Worth paying for only where the device shares the host's physical
	 * memory, in which case the mapping is what makes a transfer unnecessary.
	 */
	explicit pinned_memory_source(bool mapped) noexcept;
	~pinned_memory_source() override;

	/**
	 * @brief Check whether this source maps its memory into the device address
	 * space.
	 *
	 * @return true The memory is device-addressable.
	 * @return false The memory has to be transferred to reach a device.
	 */
	bool is_mapped() const noexcept;

	void* allocate(std::size_t size) override;
	void deallocate(void *data) noexcept override;
	bool is_host_accessible() const noexcept override;
	std::size_t get_base_alignment() const noexcept override;

private:
	bool m_mapped;
};

} // namespace cuda
} // namespace rexlib
