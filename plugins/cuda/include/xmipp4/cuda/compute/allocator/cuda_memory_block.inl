// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_memory_block.hpp"

#include <xmipp4/core/memory/align.hpp>

namespace xmipp4
{
namespace compute
{

inline
cuda_memory_block::cuda_memory_block(void *data, 
                                     std::size_t size, 
                                     const cuda_device_queue *queue ) noexcept
    : cuda_memory_block(data, memory::get_alignment(data), size, queue)
{
}

inline
cuda_memory_block::cuda_memory_block(void *data, 
                                     std::size_t alignment,
                                     std::size_t size, 
                                     const cuda_device_queue *queue ) noexcept
    : m_data(data)
    , m_alignment(alignment)
    , m_size(size)
    , m_queue(queue)
{
}


inline
void* cuda_memory_block::get_data() const noexcept
{
    return m_data;
}

inline
std::size_t cuda_memory_block::get_alignment() const noexcept
{
    return m_alignment;
}

inline
std::size_t cuda_memory_block::get_size() const noexcept
{
    return m_size;
}

inline
const cuda_device_queue* cuda_memory_block::get_queue() const noexcept
{
    return m_queue;
}



inline
bool cuda_memory_block_less::operator()(const cuda_memory_block &lhs, 
                                        const cuda_memory_block &rhs ) const noexcept
{
    bool result;

    if (lhs.get_queue() < rhs.get_queue())
    {
        result = true;
    }
    else if (lhs.get_queue() == rhs.get_queue())
    {
        if (lhs.get_size() < rhs.get_size())
        {
            result = true;
        }
        else if (lhs.get_size() == rhs.get_size())
        {
            if(lhs.get_alignment() < rhs.get_alignment())
            {
                result = true;
            }
            else if (lhs.get_alignment() == rhs.get_alignment())
            {
                result = lhs.get_data() < rhs.get_data();
            }
            else // lhs.get_alignment() > rhs.get_alignment()
            {
                result = false;
            }
        }
        else // lhs.get_size() > rhs.get_size()
        {
            result = false;
        }
    }
    else // lhs.get_queue_id() > rhs.get_queue_id()
    {
        result = false;
    }

    return result;
}

} // namespace compute
} // namespace xmipp4
