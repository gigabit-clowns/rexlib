// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <rexlib/em/image/mapped_file_image_scratch_storage.hpp>

#include "fixtures/scoped_path.hpp"

#include <rexlib/em/image/exceptions/image_file_error.hpp>
#include <rexlib/em/image/image_scratch_storage.hpp>

#include <boost/filesystem/operations.hpp>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

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

void write_file(const std::string &path, const std::string &text)
{
	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output << text;
}

// What a storage holds, as text.
std::string read_storage(const image_scratch_storage &storage)
{
	const auto *data = storage.get_data();
	return std::string(reinterpret_cast<const char*>(data), storage.get_size());
}

} // anonymous namespace

TEST_CASE(
	"create_mapped_file_image_scratch_storage maps a file of the size it is "
	"given",
	"[mapped_file_image_scratch_storage]"
)
{
	const scoped_path path("mapped_file_scratch_storage_size.mapped");

	const auto storage =
		create_mapped_file_image_scratch_storage(path.get(), 4096);

	REQUIRE( storage != nullptr );

	SECTION( "the storage has that size" )
	{
		CHECK( storage->get_size() == 4096 );
	}

	SECTION( "so does its file" )
	{
		CHECK( boost::filesystem::file_size(path.get()) == 4096 );
	}

	SECTION( "its memory can be reached" )
	{
		const image_scratch_storage &const_storage = *storage;

		REQUIRE( storage->get_data() != nullptr );
		CHECK( const_storage.get_data() == storage->get_data() );
	}
}

TEST_CASE(
	"what is written through a mapped file scratch storage is in its file",
	"[mapped_file_image_scratch_storage]"
)
{
	const scoped_path path("mapped_file_scratch_storage_write.mapped");
	const std::string text = "what a scratch holds";
	auto storage =
		create_mapped_file_image_scratch_storage(path.get(), text.size());
	std::memcpy(storage->get_data(), text.data(), text.size());

	SECTION( "while the storage is alive" )
	{
		CHECK( read_file(path.get()) == text );
	}

	SECTION( "and after it is destroyed, since the file is not removed" )
	{
		storage.reset();

		REQUIRE( boost::filesystem::exists(path.get()) );
		CHECK( read_file(path.get()) == text );
	}
}

TEST_CASE(
	"create_mapped_file_image_scratch_storage keeps what a file already at "
	"its path holds",
	"[mapped_file_image_scratch_storage]"
)
{
	const scoped_path path("mapped_file_scratch_storage_keep.mapped");
	const std::string text = "left by another run";
	write_file(path.get(), text);

	SECTION( "all of it when the size is that of the file" )
	{
		const auto storage =
			create_mapped_file_image_scratch_storage(path.get(), text.size());

		CHECK( read_storage(*storage) == text );
		CHECK( read_file(path.get()) == text );
	}

	SECTION( "the start of it when the size is smaller" )
	{
		const auto storage =
			create_mapped_file_image_scratch_storage(path.get(), 4);

		CHECK( boost::filesystem::file_size(path.get()) == 4 );
		CHECK( read_storage(*storage) == text.substr(0, 4) );
	}

	SECTION( "followed by more room when the size is larger" )
	{
		const auto storage =
			create_mapped_file_image_scratch_storage(path.get(), 64);

		CHECK( boost::filesystem::file_size(path.get()) == 64 );
		CHECK( read_storage(*storage).substr(0, text.size()) == text );
	}
}

TEST_CASE(
	"create_mapped_file_image_scratch_storage refuses a file it can not map",
	"[mapped_file_image_scratch_storage]"
)
{
	SECTION( "a file of no bytes" )
	{
		const scoped_path path("mapped_file_scratch_storage_empty.mapped");

		REQUIRE_THROWS_AS(
			create_mapped_file_image_scratch_storage(path.get(), 0),
			image_file_error
		);
		CHECK_FALSE( boost::filesystem::exists(path.get()) );
	}

	SECTION( "a file in a directory that does not exist" )
	{
		const scoped_path path(
			"mapped_file_scratch_storage_absent/values.mapped"
		);

		REQUIRE_THROWS_MATCHES(
			create_mapped_file_image_scratch_storage(path.get(), 64),
			image_file_error,
			Catch::Matchers::MessageMatches(
				Catch::Matchers::StartsWith(path.get() + ": ")
			)
		);
	}
}

TEST_CASE(
	"scratch storages that map the same file share its contents",
	"[mapped_file_image_scratch_storage]"
)
{
	const scoped_path path("mapped_file_scratch_storage_shared.mapped");
	const std::string first_text = "from the first";
	const std::string second_text = "by the second";
	const auto first =
		create_mapped_file_image_scratch_storage(path.get(), 32);
	std::memcpy(first->get_data(), first_text.data(), first_text.size());

	const auto second =
		create_mapped_file_image_scratch_storage(path.get(), 32);

	SECTION( "what one wrote before the other was created" )
	{
		CHECK( read_storage(*second).substr(0, first_text.size()) ==
			first_text );
	}

	SECTION( "and what one writes afterwards" )
	{
		std::memcpy(
			second->get_data(),
			second_text.data(),
			second_text.size()
		);

		CHECK( read_storage(*first).substr(0, second_text.size()) ==
			second_text );
	}
}
