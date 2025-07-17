// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/cuda/compute/allocator/cuda_memory_block_pool.hpp>

#include <cstddef>

#include <catch2/catch_test_macros.hpp>
#include <trompeloeil.hpp>

using namespace xmipp4;
using namespace xmipp4::compute;

class mock_allocator
{
public:
    MAKE_MOCK1(allocate, void* (std::size_t), const);
    MAKE_MOCK2(deallocate, void (void*, std::size_t), const);

};

static 
cuda_memory_block_pool::iterator 
generate_partitions(cuda_memory_block_pool &pool, 
                    std::size_t number,
                    std::uintptr_t address = 0xDEADBEEF,
                    std::size_t block_size = 1024,
                    const cuda_device_queue *queue = nullptr,
                    bool free = false )
{
    cuda_memory_block_pool::iterator ite;
    cuda_memory_block_pool::iterator result;
    const cuda_memory_block_pool::iterator null;

    for (std::size_t i = 0; i < number; ++i)
    {
        std::tie(ite, std::ignore) = pool.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(
                reinterpret_cast<void*>(address), 
                block_size, 
                queue
            ),
            std::forward_as_tuple(ite, null, free)
        );
        update_backward_link(ite);
        address += block_size;

        if (i == 0)
        {
            result = ite;
        }
    }

    return result;
}

static void invalidate_forward_link(cuda_memory_block_pool::iterator ite)
{
    const cuda_memory_block_pool::iterator null;
    ite->second.get_next_block()->second.set_previous_block(null);
}

static void invalidate_backward_link(cuda_memory_block_pool::iterator ite)
{
    const cuda_memory_block_pool::iterator null;
    ite->second.get_previous_block()->second.set_next_block(null);
}

static const cuda_device_queue* make_phantom_queue(std::uintptr_t i)
{
    return reinterpret_cast<const cuda_device_queue*>(i);
}

static void populate_test_pool(cuda_memory_block_pool &pool)
{
    const cuda_memory_block_pool::iterator null;
    const auto queue1 = reinterpret_cast<const cuda_device_queue*>(std::uintptr_t(1));
    const auto queue2 = reinterpret_cast<const cuda_device_queue*>(std::uintptr_t(2));
    const auto queue3 = reinterpret_cast<const cuda_device_queue*>(std::uintptr_t(3));

    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(96)), 32, queue1), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(128)), 64, queue1), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(192)), 64, queue1), 
        cuda_memory_block_context(null, null, false)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(256)), 32, queue2), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(288)), 64, queue2), 
        cuda_memory_block_context(null, null, false)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(352)), 64, queue2), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(416)), 96, queue2), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(512)), 64, queue2), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(576)), 64, queue3), 
        cuda_memory_block_context(null, null, true)
    );
    pool.emplace(
        cuda_memory_block(reinterpret_cast<void*>(std::uintptr_t(640)), 128, queue3), 
        cuda_memory_block_context(null, null, false)
    );
}



TEST_CASE( "is_partition should return false when not partitioned", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    REQUIRE( is_partition(ite->second) == false );
}

TEST_CASE( "is_partition should return true when partitioned in two", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);

    // Check for both halves that the condition is satisfied
    REQUIRE( is_partition(ite->second) == true );
    ite = ite->second.get_next_block();
    REQUIRE( is_partition(ite->second) == true );
}

TEST_CASE( "is_partition should return true when partitioned in three", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 3);

    // Check for all thirds that the condition is satisfied
    REQUIRE( is_partition(ite->second) == true );
    ite = ite->second.get_next_block();
    REQUIRE( is_partition(ite->second) == true );
    ite = ite->second.get_next_block();
    REQUIRE( is_partition(ite->second) == true );
}

TEST_CASE( "is_mergeable should return false when null iterator", "[cuda_memory_block_pool]" )
{
    const cuda_memory_block_pool::iterator null;
    REQUIRE( is_mergeable(null) == false );
}

TEST_CASE( "is_mergeable should return false when occupied", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);
    ite->second.set_free(false);
    REQUIRE( is_mergeable(ite) == false );
}

TEST_CASE( "is_mergeable should return true when free", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);
    ite->second.set_free(true);
    REQUIRE( is_mergeable(ite) == true );
}

TEST_CASE( "update_forward_link should produce valid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    invalidate_forward_link(ite);

    REQUIRE( check_forward_link(ite) == false );
    update_forward_link(ite);
    REQUIRE( check_forward_link(ite) == true );
}

TEST_CASE( "update_backward_link should produce valid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite = ite->second.get_next_block();
    invalidate_backward_link(ite);

    REQUIRE( check_backward_link(ite) == false );
    update_backward_link(ite);
    REQUIRE( check_backward_link(ite) == true );
}

TEST_CASE( "update_links should produce valid links", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 3);
    ite = ite->second.get_next_block();
    invalidate_forward_link(ite);
    invalidate_backward_link(ite);

    REQUIRE( check_forward_link(ite) == false );
    REQUIRE( check_backward_link(ite) == false );
    update_links(ite);
    REQUIRE( check_forward_link(ite) == true );
    REQUIRE( check_backward_link(ite) == true );
}

TEST_CASE( "check_forward_link should return false with invalid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    invalidate_forward_link(ite);

    REQUIRE( check_forward_link(ite) == false );
}

TEST_CASE( "check_forward_link should return true with valid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);

    REQUIRE( check_forward_link(ite) == true );
}

TEST_CASE( "check_forward_link should return true with empty link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    REQUIRE( check_forward_link(ite) == true );
}

TEST_CASE( "check_backward_link should return false with invalid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite = ite->second.get_next_block();
    invalidate_backward_link(ite);

    REQUIRE( check_backward_link(ite) == false );
}

TEST_CASE( "check_backward_link should return true with valid link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite = ite->second.get_next_block();
    
    REQUIRE( check_backward_link(ite) == true );
}

TEST_CASE( "check_backward_link should return true with empty link", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    REQUIRE( check_backward_link(ite) == true );
}

TEST_CASE( "find_suitable_block should return a properly aligned free block larger than the requested size", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    populate_test_pool(pool);
    
    cuda_memory_block_pool::iterator ite;
    ite = find_suitable_block(pool, 64, 32, make_phantom_queue(2));
    REQUIRE( reinterpret_cast<std::uintptr_t>(ite->first.get_data()) == 352 );
    ite = find_suitable_block(pool, 64, 512, make_phantom_queue(2));
    REQUIRE( reinterpret_cast<std::uintptr_t>(ite->first.get_data()) == 512 );
}

TEST_CASE( "find_suitable_block should return end when no suitable block is found", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    populate_test_pool(pool);

    REQUIRE( find_suitable_block(pool, 32, 1, make_phantom_queue(4)) == pool.end() ); // Non-existent queue_id
    REQUIRE( find_suitable_block(pool, 128, 1, make_phantom_queue(3)) == pool.end() ); // This queue_id does not contain a large enough block
}

TEST_CASE( "consider partitioning block should not partition with reminder is smaller than threshold", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    consider_partitioning_block(pool, ite, 768, 512);
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "consider partitioning block should partition with reminder is greater or equal than threshold", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    SECTION("equal")
    {
        consider_partitioning_block(pool, ite, 768, 256);
        REQUIRE( pool.size() == 2 );
    }
    SECTION("greater")
    {
        consider_partitioning_block(pool, ite, 768, 64);
        REQUIRE( pool.size() == 2 );
    }
}

TEST_CASE( "partition_block should output two valid partitions", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    const cuda_memory_block_pool::iterator null;
    const std::uintptr_t address = 0xDEADBEEF;
    const std::size_t size = 1024;
    const auto queue = make_phantom_queue(2);
    auto ite = generate_partitions(pool, 1, address, size, queue);

    ite = partition_block(pool, ite, 768, 256);
    REQUIRE( pool.size() == 2 );

    REQUIRE( reinterpret_cast<std::uintptr_t>(ite->first.get_data()) == 0xDEADBEEF );
    REQUIRE( ite->first.get_size() == 768 );
    REQUIRE( ite->first.get_queue() == queue );
    REQUIRE( ite->second.get_previous_block() == null );
    REQUIRE( ite->second.is_free() == false );
    REQUIRE( check_forward_link(ite) == true );

    ite = ite->second.get_next_block();
    REQUIRE( reinterpret_cast<std::uintptr_t>(ite->first.get_data()) == (0xDEADBEEF + 768) );
    REQUIRE( ite->first.get_size() == 256 );
    REQUIRE( ite->first.get_queue() == queue );
    REQUIRE( ite->second.get_next_block() == null );
    REQUIRE( ite->second.is_free() == false );
    REQUIRE( check_backward_link(ite) == true );
}

TEST_CASE( "consider_merging_block should not merge when is not partition", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);

    consider_merging_block(pool, ite);
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "consider_merging_block should not merge when prev is occupied", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite->second.set_free(false);
    ite = ite->second.get_next_block();

    const auto old_ite = ite;
    consider_merging_block(pool, ite);
    REQUIRE( ite == old_ite );
    REQUIRE( pool.size() == 2 );
}

TEST_CASE( "consider_merging_block should not merge when next is occupied", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite->second.get_next_block()->second.set_free(false);

    const auto old_ite = ite;
    consider_merging_block(pool, ite);
    REQUIRE( ite == old_ite );
    REQUIRE( pool.size() == 2 );
}

TEST_CASE( "consider_merging_block should merge when prev is free", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite->second.set_free(true);
    ite = ite->second.get_next_block();
    ite->second.set_free(true);

    consider_merging_block(pool, ite);
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "consider_merging_block should merge when next is free", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 2);
    ite->second.set_free(true);
    ite->second.get_next_block()->second.set_free(true);

    consider_merging_block(pool, ite);
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "consider_merging_block should merge when prev and next is free", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 3);
    ite->second.set_free(true);
    ite = ite->second.get_next_block();
    ite->second.set_free(true);
    ite->second.get_next_block()->second.set_free(true);

    consider_merging_block(pool, ite);
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "merge_blocks (2) produce valid blocks", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    const uintptr_t base_address = 0xDEADBEEF;
    const std::size_t block_size = 1024;
    const auto queue = make_phantom_queue(423);
    auto ite = generate_partitions(pool, 4, base_address, block_size, queue, true);

    const auto keep_left = ite;
    const auto merge1 = keep_left->second.get_next_block();
    const auto merge2 = merge1->second.get_next_block();
    const auto keep_right = merge2->second.get_next_block();

    const auto merged = merge_blocks(pool, merge1, merge2);
    REQUIRE( reinterpret_cast<std::uintptr_t>(merged->first.get_data()) == (base_address + block_size));
    REQUIRE( merged->first.get_size() == 2*block_size );
    REQUIRE( merged->first.get_queue() == queue );
    REQUIRE( merged->second.get_previous_block() == keep_left );
    REQUIRE( merged->second.get_next_block() == keep_right );
    REQUIRE( merged->second.is_free() == true );
    REQUIRE( check_links(merged) == true );
    REQUIRE( pool.size() == 3 );
}

TEST_CASE( "merge_blocks (3) produce valid blocks", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    const uintptr_t base_address = 0xDEADBEEF;
    const std::size_t block_size = 1024;
    const auto queue = make_phantom_queue(7548);
    auto ite = generate_partitions(pool, 5, base_address, block_size, queue, true);

    const auto keep_left = ite;
    const auto merge1 = keep_left->second.get_next_block();
    const auto merge2 = merge1->second.get_next_block();
    const auto merge3 = merge2->second.get_next_block();
    const auto keep_right = merge3->second.get_next_block();

    const auto merged = merge_blocks(pool, merge1, merge2, merge3);
    REQUIRE( reinterpret_cast<std::uintptr_t>(merged->first.get_data()) == (base_address + block_size));
    REQUIRE( merged->first.get_size() == 3*block_size );
    REQUIRE( merged->first.get_queue() == queue );
    REQUIRE( merged->second.get_previous_block() == keep_left );
    REQUIRE( merged->second.get_next_block() == keep_right );
    REQUIRE( merged->second.is_free() == true );
    REQUIRE( check_links(merged) == true );
    REQUIRE( pool.size() == 3 );
}

TEST_CASE( "create_block should call the allocator", "[cuda_memory_block_pool]" )
{
    const cuda_memory_block_pool::iterator null;
    cuda_memory_block_pool pool;
    mock_allocator allocator;
    auto* address = reinterpret_cast<void*>(std::uintptr_t(0xDEADBEEF));
    const auto size = 768;
    const auto queue = make_phantom_queue(485934);
    REQUIRE_CALL(allocator, allocate(size))
        .RETURN(address)
        .TIMES(1);

    const auto ite = create_block(pool, allocator, size, queue);
    REQUIRE( ite->first.get_data() == address );
    REQUIRE( ite->first.get_size() == size);
    REQUIRE( ite->first.get_queue() == queue );
    REQUIRE( ite->second.get_previous_block() == null );
    REQUIRE( ite->second.get_next_block() == null );
    REQUIRE( ite->second.is_free() == true );
    REQUIRE( pool.size() == 1 );
}

TEST_CASE( "allocate_block should return a pooled block when possible", "[cuda_memory_block_pool]" )
{
    mock_allocator allocator;
    cuda_memory_block_pool pool;
    populate_test_pool(pool);

    const auto queue = make_phantom_queue(2);
    const auto block = allocate_block(pool, allocator, 64, 32, queue, 1, 1);
    REQUIRE( block != pool.end() );
    REQUIRE( reinterpret_cast<std::uintptr_t>(block->first.get_data()) == 352 );
    REQUIRE( block->first.get_queue() == queue );
    REQUIRE( block->second.is_free() == false ); // Should mark it as occupied
}

TEST_CASE( "allocate_block should call the allocator when no block is available", "[cuda_memory_block_pool]" )
{
    mock_allocator allocator;
    cuda_memory_block_pool pool;
    populate_test_pool(pool);

    const std::size_t size = 768;
    const std::size_t size_step = 512;
    const auto queue = make_phantom_queue(2);
    const auto rounded_size = memory::align_ceil(size, size_step);
    auto* address = reinterpret_cast<void*>(std::uintptr_t(0xDEADBEEF));

    REQUIRE_CALL(allocator, allocate(rounded_size))
        .RETURN(address)
        .TIMES(1);

    const auto block = allocate_block(pool, allocator, size, 1, queue, 0, size_step);
    REQUIRE( block != pool.end() );
    REQUIRE( block->first.get_data() == address );
    REQUIRE( block->first.get_queue() == queue );
    REQUIRE( block->second.is_free() == false ); // Should mark it as occupied
}

TEST_CASE( "allocate_block should return pool.end() on failure", "[cuda_memory_block_pool]" )
{
    mock_allocator allocator;
    cuda_memory_block_pool pool;
    populate_test_pool(pool);

    const std::size_t size = 768;
    const std::size_t size_step = 512;
    const auto queue = make_phantom_queue(2);

    const auto rounded_size = memory::align_ceil(size, size_step);
    void* address = nullptr;

    REQUIRE_CALL(allocator, allocate(rounded_size))
        .RETURN(address)
        .TIMES(1);

    const auto block = allocate_block(pool, allocator, size, 1, queue, 0, size_step);
    REQUIRE( block == pool.end() );
}

TEST_CASE( "release_blocks should call the deallocator with free blocks", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);
    ite->second.set_free(true);

    mock_allocator allocator;
    auto* address = reinterpret_cast<void*>(std::uintptr_t(0xDEADBEEF));
    const auto size = 1024;
    REQUIRE_CALL(allocator, deallocate(address, size))
        .TIMES(1);
    

    release_blocks(pool, allocator);
    REQUIRE( pool.size() == 0 );
}   

TEST_CASE( "release_blocks should not call the deallocator with occupied blocks", "[cuda_memory_block_pool]" )
{
    cuda_memory_block_pool pool;
    auto ite = generate_partitions(pool, 1);
    ite->second.set_free(false);

    mock_allocator allocator;
    
    release_blocks(pool, allocator);
    REQUIRE( pool.size() == 1 );
}
