// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/host_image_scratch_storage.hpp>

#include <rexlib/core/memory/align.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE(
	"create_host_image_scratch_storage gives memory of the size it is asked "
	"for",
	"[host_image_scratch_storage]"
)
{
	// A size that is not a multiple of any alignment.
	const auto storage = create_host_image_scratch_storage(1001);

	REQUIRE( storage != nullptr );

	SECTION( "the storage has that size" )
	{
		CHECK( storage->get_size() == 1001 );
	}

	SECTION( "its memory is aligned for 64-bit integers" )
	{
		REQUIRE( storage->get_data() != nullptr );
		CHECK( is_aligned(storage->get_data(), alignof(std::uint64_t)) );
	}

	SECTION( "what is written to it is read back" )
	{
		const std::string text = "what a scratch holds";
		const image_scratch_storage &const_storage = *storage;

		std::memcpy(
			storage->get_data() + 1001 - text.size(),
			text.data(),
			text.size()
		);

		const auto *data = const_storage.get_data() + 1001 - text.size();
		CHECK( std::memcmp(data, text.data(), text.size()) == 0 );
	}
}

TEST_CASE(
	"create_host_image_scratch_storage refuses a size of zero",
	"[host_image_scratch_storage]"
)
{
	REQUIRE_THROWS_AS(
		create_host_image_scratch_storage(0),
		std::invalid_argument
	);
}
