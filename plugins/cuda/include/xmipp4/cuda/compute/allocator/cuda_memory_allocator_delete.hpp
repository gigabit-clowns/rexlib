#pragma once

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_memory_allocator_delete.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Defines the cuda_memory_allocator_delete.hpp class
 * @date 2024-11-29
 * 
 */

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
