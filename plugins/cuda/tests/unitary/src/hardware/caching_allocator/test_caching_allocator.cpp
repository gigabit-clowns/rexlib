// SPDX-License-Identifier: GPL-3.0-only

#include "test_caching_allocator.hpp"

#include "test_arena.hpp"

namespace xmipp4
{
namespace cuda
{

caching_allocator_fixture::caching_allocator_fixture(bool host_accessible)
	: m_blocks(test_arena::unlimited, host_accessible)
	, m_allocator(m_resource, m_blocks.get_shared())
{
}

caching_allocator_fixture::~caching_allocator_fixture() = default;

caching_memory_allocator& caching_allocator_fixture::get() noexcept
{
	return m_allocator;
}

block_allocator_fixture& caching_allocator_fixture::get_blocks() noexcept
{
	return m_blocks;
}

const mock_memory_resource&
caching_allocator_fixture::get_resource() const noexcept
{
	return m_resource;
}

} // namespace cuda
} // namespace xmipp4
