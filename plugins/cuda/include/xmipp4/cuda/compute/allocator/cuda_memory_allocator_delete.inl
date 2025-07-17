// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_memory_allocator_delete.hpp"

namespace xmipp4
{
namespace compute
{

template <typename Allocator>
inline
cuda_memory_allocator_delete<Allocator>
::cuda_memory_allocator_delete(allocator_type& allocator) noexcept
    : m_allocator(allocator)
{
}

template <typename Allocator>
inline
void cuda_memory_allocator_delete<Allocator>
::operator()(const cuda_memory_block *block) const
{
    if (block)
    {
        m_allocator.get().deallocate(*block);
    }
}

} // namespace compute
} // namespace xmipp4
