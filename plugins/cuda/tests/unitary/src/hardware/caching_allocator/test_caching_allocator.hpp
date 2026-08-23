// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "mock/mock_memory_resource.hpp"
#include "test_block_allocator.hpp"

#include <hardware/caching_allocator/caching_memory_allocator.hpp>

namespace xmipp4
{
namespace cuda
{

/**
 * @brief An allocator wrapping the blocks of a mocked block allocator.
 *
 * All the caching is one layer down and tested there. What is left up here is
 * whether a block is turned into the right buffer, and whether dropping that
 * buffer gives the block back.
 */
class caching_allocator_fixture
{
public:
	/**
	 * @brief Build an allocator.
	 *
	 * @param host_accessible Whether the host can address what it hands out.
	 */
	explicit caching_allocator_fixture(bool host_accessible = false);
	caching_allocator_fixture(
		const caching_allocator_fixture &other
	) = delete;
	caching_allocator_fixture(caching_allocator_fixture &&other) = delete;
	~caching_allocator_fixture();

	caching_allocator_fixture&
	operator=(const caching_allocator_fixture &other) = delete;
	caching_allocator_fixture&
	operator=(caching_allocator_fixture &&other) = delete;

	caching_memory_allocator& get() noexcept;
	block_allocator_fixture& get_blocks() noexcept;
	const mock_memory_resource& get_resource() const noexcept;

private:
	mock_memory_resource m_resource;
	block_allocator_fixture m_blocks;
	caching_memory_allocator m_allocator;
};

} // namespace cuda
} // namespace xmipp4
