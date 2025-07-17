// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <xmipp4/core/span.hpp>

#include <cstddef>

namespace xmipp4 
{
namespace compute
{

class cuda_device_queue;

/**
 * @brief Represents a chunk of data managed by cuda_memory_cache.
 * 
 * It contains an unique id to a queue where this data is synchronous.
 * It also contains the size of the referenced block and a pointer 
 * to its data.
 * 
 */
class cuda_memory_block
{
public:
    /**
     * @brief Construct a new cuda memory block from its components.
     * 
     * @param data Referenced data.
     * @param size Number of bytes referenced.
     * @param queue Queue where this belongs.
     */
    cuda_memory_block(void *data, 
                      std::size_t size, 
                      const cuda_device_queue *queue ) noexcept;
    /**
     * @brief Construct a new cuda memory block from its components.
     * 
     * @param data Referenced data.
     * @param alignment The alignment of the data pointer.
     * @param size Number of bytes referenced.
     * @param queue Queue where this belongs.
     */
    cuda_memory_block(void *data, 
                      std::size_t alignment,  
                      std::size_t size, 
                      const cuda_device_queue *queue ) noexcept;
    cuda_memory_block(const cuda_memory_block &other) = default;
    cuda_memory_block(cuda_memory_block &&other) = default;
    ~cuda_memory_block() = default;

    cuda_memory_block& operator=(const cuda_memory_block &other) = default;
    cuda_memory_block& operator=(cuda_memory_block &&other) = default;

    /**
     * @brief Obtain the pointer to the data.
     * 
     * @return void* The data.
     * 
     */
    void* get_data() const noexcept;

    /**
     * @brief Get the alignment of the data pointer.
     * 
     * @return std::size_t The alignment in bytes.
     */
    std::size_t get_alignment() const noexcept;

    /**
     * @brief Get the number of bytes referenced by this object.
     * 
     * @return std::size_t Number of bytes.
     */
    std::size_t get_size() const noexcept;

    /**
     * @brief Get the queue where this block belongs to.
     * 
     * @return const cuda_device_queue* Pointer to the queue.
     */
    const cuda_device_queue* get_queue() const noexcept;

private:
    void *m_data;
    std::size_t m_alignment;
    std::size_t m_size;
    const cuda_device_queue *m_queue;

}; 



/**
 * @brief Lexicographically compare two cuda_memory_block objects.
 * 
 * First, queue IDs are compared.
 * If equal, then, sizes are compared.
 * If equal, then alignments are compared.
 * If equal, data pointers are compared.
 * 
 */
struct cuda_memory_block_less
{
    bool operator()(const cuda_memory_block &lhs, 
                    const cuda_memory_block &rhs ) const noexcept;
};

} // namespace compute
} // namespace xmipp4

#include "cuda_memory_block.inl"
