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
 * @file cuda_memory_block.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Implementation of cuda_memory_block.hpp
 * @date 2024-11-06
 * 
 */

#include "cuda_memory_block.hpp"

namespace xmipp4
{
namespace compute
{

inline
cuda_memory_block::cuda_memory_block(void *data, std::size_t size) noexcept
    : m_data(data)
    , m_size(size)
{
}

inline
void cuda_memory_block::set_data(void *data) noexcept
{
    m_data = data;
}

inline
void* cuda_memory_block::get_data() const noexcept
{
    return m_data;
}

inline
void cuda_memory_block::set_size(std::size_t size) noexcept
{
    m_size = size;
}

inline
std::size_t cuda_memory_block::get_size() const noexcept
{
    return m_size;
}



inline
bool cuda_memory_block_size_less::operator()(const cuda_memory_block &lhs, 
                                             const cuda_memory_block &rhs ) const noexcept
{
    bool result;

    if (lhs.get_size() < rhs.get_size())
    {
        result = true;
    }
    else if (lhs.get_size() == rhs.get_size())
    {
        result = lhs.get_data() < rhs.get_data();
    }
    else // lhs.get_size() > rhs.get_size()
    {
        result = false;
    }

    return result;
}

inline
bool cuda_memory_block_size_less::operator()(std::size_t lhs, 
                                             const cuda_memory_block &rhs ) const noexcept
{
    return lhs < rhs.get_size();
}

inline
bool cuda_memory_block_size_less::operator()(const cuda_memory_block &lhs, 
                                             std::size_t rhs ) const noexcept
{
    return lhs.get_size() < rhs;
}

} // namespace compute
} // namespace xmipp4
