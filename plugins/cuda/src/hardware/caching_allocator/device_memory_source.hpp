// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "memory_source.hpp"

namespace xmipp4
{
namespace cuda
{

/**
 * @brief Device-local memory, as obtained from cudaMalloc.
 */
class device_memory_source final
	: public memory_source
{
public:
	/**
	 * @brief Construct a source drawing from a given device.
	 *
	 * @param ordinal Ordinal of the device the memory belongs to.
	 */
	explicit device_memory_source(int ordinal) noexcept;
	~device_memory_source() override;

	/**
	 * @brief Get the device this source draws from.
	 *
	 * @return int The device ordinal.
	 */
	int get_ordinal() const noexcept;

	void* allocate(std::size_t size) override;
	void deallocate(void *data) noexcept override;
	bool is_host_accessible() const noexcept override;
	std::size_t get_base_alignment() const noexcept override;

private:
	int m_ordinal;
};

} // namespace cuda
} // namespace xmipp4
