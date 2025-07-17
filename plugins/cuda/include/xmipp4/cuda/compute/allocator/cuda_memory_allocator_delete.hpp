// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <functional>

namespace xmipp4
{
namespace compute
{

class cuda_memory_block;

/**
 * @brief Deleter to be able to use custom cuda allocators with
 * smart pointers.
 * 
 * @tparam Allocator Concrete type of the allocator.
 */
template <typename Allocator>
class cuda_memory_allocator_delete
{
public:
    using allocator_type = Allocator;

    explicit cuda_memory_allocator_delete(allocator_type& allocator) noexcept;
    cuda_memory_allocator_delete(const cuda_memory_allocator_delete &other) = default;
    cuda_memory_allocator_delete(cuda_memory_allocator_delete &&other) = default;
    ~cuda_memory_allocator_delete() = default;

    cuda_memory_allocator_delete&
    operator=(const cuda_memory_allocator_delete &other) = default;
    cuda_memory_allocator_delete&
    operator=(cuda_memory_allocator_delete &&other) = default;

    /**
     * @brief Deallocate the provided block.
     * 
     * @param block Block to be deallocated. May be null.
     */
    void operator()(const cuda_memory_block *block) const;

private:
    std::reference_wrapper<allocator_type> m_allocator;

};

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_allocator_delete.inl"
