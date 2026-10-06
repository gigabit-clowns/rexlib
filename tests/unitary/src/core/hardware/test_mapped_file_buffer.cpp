// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/core/hardware/mapped_file_buffer.hpp>

#include "../../em/image/fixtures/scoped_path.hpp"

#include <rexlib/core/exceptions/file_error.hpp>
#include <rexlib/core/hardware/buffer.hpp>
#include <rexlib/core/hardware/memory_resource.hpp>

#include <boost/filesystem/operations.hpp>

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

using namespace rexlib;

namespace
{

std::string read_file(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);

	return std::string(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>()
	);
}

} // anonymous namespace

TEST_CASE(
	"create_mapped_file_buffer maps a file of the size it is given",
	"[mapped_file_buffer]"
)
{
	const scoped_path path("mapped_file_buffer_size.mapped");

	const auto mapped = create_mapped_file_buffer(path.get(), 4096);

	REQUIRE( mapped != nullptr );

	SECTION( "the buffer has that size" )
	{
		CHECK( mapped->get_size() == 4096 );
	}

	SECTION( "so does its file" )
	{
		CHECK( boost::filesystem::file_size(path.get()) == 4096 );
	}

	SECTION( "its memory is host memory" )
	{
		const buffer &const_mapped = *mapped;

		REQUIRE( mapped->get_host_ptr() != nullptr );
		CHECK( const_mapped.get_host_ptr() == mapped->get_host_ptr() );
		CHECK( &mapped->get_memory_resource() == &get_host_memory_resource() );
	}
}

TEST_CASE(
	"what is written through a mapped file buffer is in its file",
	"[mapped_file_buffer]"
)
{
	const scoped_path path("mapped_file_buffer_write.mapped");
	const std::string text = "what a scratch holds";

	auto mapped = create_mapped_file_buffer(path.get(), text.size());
	std::memcpy(mapped->get_host_ptr(), text.data(), text.size());

	SECTION( "while the buffer is alive" )
	{
		CHECK( read_file(path.get()) == text );
	}

	SECTION( "and after it is destroyed, since the file is not removed" )
	{
		mapped.reset();

		REQUIRE( boost::filesystem::exists(path.get()) );
		CHECK( read_file(path.get()) == text );
	}
}

TEST_CASE(
	"create_mapped_file_buffer replaces a file already at its path",
	"[mapped_file_buffer]"
)
{
	const scoped_path path("mapped_file_buffer_replace.mapped");
	{
		std::ofstream output(
			path.get().c_str(),
			std::ios::out | std::ios::binary
		);
		output << "left over from another run";
	}

	const auto mapped = create_mapped_file_buffer(path.get(), 8);

	CHECK( mapped->get_size() == 8 );
	CHECK( boost::filesystem::file_size(path.get()) == 8 );
}

TEST_CASE(
	"create_mapped_file_buffer refuses a file it can not map",
	"[mapped_file_buffer]"
)
{
	SECTION( "a file of no bytes" )
	{
		const scoped_path path("mapped_file_buffer_empty.mapped");

		REQUIRE_THROWS_AS(
			create_mapped_file_buffer(path.get(), 0),
			std::invalid_argument
		);
		CHECK_FALSE( boost::filesystem::exists(path.get()) );
	}

	SECTION( "a file in a directory that does not exist" )
	{
		const scoped_path path("mapped_file_buffer_absent/values.mapped");

		REQUIRE_THROWS_MATCHES(
			create_mapped_file_buffer(path.get(), 64),
			file_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(
					path.get() + ": create_mapped_file_buffer: "
				)
			)
		);
	}
}
