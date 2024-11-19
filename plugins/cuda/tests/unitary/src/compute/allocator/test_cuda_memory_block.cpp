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
 * @file test_cuda_memory_block.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Tests for cuda_memory_block.hpp
 * @date 2024-11-19
 * 
 */

#include <xmipp4/cuda/compute/allocator/cuda_memory_block.hpp>

#include <cstddef>

#include <catch2/catch_test_macros.hpp>

using namespace xmipp4::compute;

TEST_CASE( "construct cuda_memory_block", "[cuda_memory_block]" )
{
    const std::uintptr_t ptr_value = 0xDEADBEEF;
    auto *const ptr = reinterpret_cast<void*>(ptr_value);

    cuda_memory_block block(ptr, 0xC0FFE, 1357);
    REQUIRE( block.get_data() == ptr );
    REQUIRE( block.get_size() == 0xC0FFE );
    REQUIRE( block.get_queue_id() == 1357 );
}
