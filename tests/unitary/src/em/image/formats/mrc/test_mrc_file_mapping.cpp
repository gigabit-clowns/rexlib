// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <em/image/formats/mrc/mrc_file_mapping.hpp>

#include <rexlib/em/image/exceptions/image_format_error.hpp>
#include <rexlib/tests/assets.hpp>

#include <boost/filesystem/operations.hpp>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using namespace rexlib::em::mrc;

namespace
{

// A path under the build tree that is removed when the test leaves, whether
// it succeeded or not.
//
// The scratch directory is shared and every case runs as a process of its
// own, so two cases naming the same file race: one truncates what the other
// has mapped. No two names here may repeat.
class scoped_path
{
public:
	explicit scoped_path(const std::string &name)
		: m_path(get_scratch_path(name))
	{
		std::remove(m_path.c_str());
	}

	scoped_path(const scoped_path &other) = delete;
	scoped_path(scoped_path &&other) = delete;

	~scoped_path()
	{
		std::remove(m_path.c_str());
	}

	scoped_path& operator=(const scoped_path &other) = delete;
	scoped_path& operator=(scoped_path &&other) = delete;

	const std::string& get() const noexcept
	{
		return m_path;
	}

private:
	std::string m_path;
};

void write_file(const std::string &path, const std::vector<char> &contents)
{
	std::ofstream output(path.c_str(), std::ios::out | std::ios::binary);
	output.write(contents.data(), static_cast<std::streamsize>(
		contents.size()));
}

std::vector<char> read_file(const std::string &path)
{
	std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
	return std::vector<char>(
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>()
	);
}

std::size_t size_on_disk(const std::string &path)
{
	return static_cast<std::size_t>(boost::filesystem::file_size(path));
}

} // anonymous namespace

TEST_CASE( "a file is laid out in full before it is mapped",
	"[mrc_file_mapping]" )
{
	const scoped_path path("mapping_layout.tmp");

	SECTION( "it is created at exactly the size asked for" )
	{
		create_file(path.get(), 2048);

		REQUIRE( size_on_disk(path.get()) == 2048 );
	}

	SECTION( "a file already there is replaced" )
	{
		write_file(path.get(), std::vector<char>(8192, 'x'));
		create_file(path.get(), 1024);

		REQUIRE( size_on_disk(path.get()) == 1024 );
	}

	SECTION( "a file of no bytes is refused" )
	{
		REQUIRE_THROWS_AS( create_file(path.get(), 0), image_format_error );
	}
}

TEST_CASE( "an MRC file is read through its mapping",
	"[mrc_file_mapping]" )
{
	const scoped_path path("mapping_read.tmp");

	SECTION( "the whole file is mapped from its first byte" )
	{
		write_file(path.get(), {'M', 'A', 'P', ' ', '\1', '\2'});

		const mrc_file_mapping mapping(path.get(), read_only);

		REQUIRE( mapping.get_size() == 6 );
		REQUIRE( as_uint8(mapping.get_data()[0]) ==
			static_cast<std::uint8_t>('M') );
		REQUIRE( as_uint8(mapping.get_data()[5]) == 2 );
	}

	SECTION( "a file that is not there is refused" )
	{
		REQUIRE_THROWS_AS(
			mrc_file_mapping(path.get(), read_only),
			image_format_error
		);
	}

	SECTION( "an empty file is refused" )
	{
		write_file(path.get(), {});

		REQUIRE_THROWS_AS(
			mrc_file_mapping(path.get(), read_only),
			image_format_error
		);
	}
}

TEST_CASE( "an MRC file is written through its mapping",
	"[mrc_file_mapping]" )
{
	const scoped_path path("mapping_write.tmp");

	SECTION( "what is written reaches the storage" )
	{
		create_file(path.get(), 4);

		{
			mrc_file_mapping mapping(path.get(), read_write);
			auto *data = mapping.get_data();
			data[0] = as_byte('M');
			data[1] = as_byte('A');
			data[2] = as_byte('P');
			data[3] = as_byte(' ');
			mapping.flush();
		}

		const auto contents = read_file(path.get());

		REQUIRE( contents == std::vector<char>{'M', 'A', 'P', ' '} );
	}

	SECTION( "a second mapping sees what the first one wrote" )
	{
		create_file(path.get(), 2);

		mrc_file_mapping writer(path.get(), read_write);
		writer.get_data()[0] = as_byte(0x42);
		writer.flush();

		const mrc_file_mapping reader(path.get(), read_only);

		REQUIRE( as_uint8(reader.get_data()[0]) == 0x42 );
	}
}

TEST_CASE( "a mapping carries its file when it is moved",
	"[mrc_file_mapping]" )
{
	const scoped_path path("mapping_moved.tmp");
	write_file(path.get(), {'a', 'b', 'c', 'd'});

	SECTION( "a moved mapping keeps reading the file" )
	{
		mrc_file_mapping original(path.get(), read_only);
		const mrc_file_mapping moved(std::move(original));

		REQUIRE( moved.get_size() == 4 );
		REQUIRE( as_uint8(moved.get_data()[0]) ==
			static_cast<std::uint8_t>('a') );
	}

	SECTION( "a mapping assigned over keeps reading the file" )
	{
		const scoped_path other("mapping-other.tmp");
		write_file(other.get(), {'z'});

		mrc_file_mapping original(path.get(), read_only);
		mrc_file_mapping target(other.get(), read_only);
		target = std::move(original);

		REQUIRE( target.get_size() == 4 );
		REQUIRE( as_uint8(target.get_data()[0]) ==
			static_cast<std::uint8_t>('a') );
	}
}
