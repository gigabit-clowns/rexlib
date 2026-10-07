// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/image_scratch_storage_buffer.hpp>

#include "mock/mock_image_scratch_storage.hpp"

#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>
#include <rexlib/core/memory/byte.hpp>

#include <array>
#include <cstddef>
#include <memory>
#include <trompeloeil.hpp>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"an image_scratch_storage_buffer is a buffer over its storage",
	"[image_scratch_storage_buffer]"
)
{
	std::array<rexlib::byte, 16> memory = {};
	const auto storage = std::make_shared<mock_image_scratch_storage>();
	const mock_image_scratch_storage &const_storage = *storage;
	image_scratch_storage_buffer memory_buffer(storage);
	const buffer &const_buffer = memory_buffer;

	SECTION( "its memory is that of the storage" )
	{
		REQUIRE_CALL(*storage, get_data()).LR_RETURN(memory.data());

		CHECK( memory_buffer.get_host_ptr() == memory.data() );
	}

	SECTION( "also when it is only read" )
	{
		REQUIRE_CALL(const_storage, get_data()).LR_RETURN(memory.data());

		CHECK( const_buffer.get_host_ptr() == memory.data() );
	}

	SECTION( "its size is that of the storage" )
	{
		REQUIRE_CALL(*storage, get_size()).RETURN(16);

		CHECK( memory_buffer.get_size() == 16 );
	}

	SECTION( "its memory is host memory" )
	{
		CHECK( &memory_buffer.get_memory_resource() ==
			&get_host_memory_resource() );
	}
}

TEST_CASE(
	"an image_scratch_storage_buffer keeps its storage alive",
	"[image_scratch_storage_buffer]"
)
{
	auto storage = std::make_shared<mock_image_scratch_storage>();
	const std::weak_ptr<mock_image_scratch_storage> watched = storage;

	auto memory_buffer =
		std::make_shared<image_scratch_storage_buffer>(std::move(storage));

	REQUIRE_FALSE( watched.expired() );

	memory_buffer.reset();

	CHECK( watched.expired() );
}
