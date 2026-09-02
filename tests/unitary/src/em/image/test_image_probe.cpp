// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/image_probe.hpp>

#include <rexlib/core/memory/byte.hpp>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

class scoped_file
{
public:
	explicit scoped_file(std::string path, std::size_t size)
		: m_path(std::move(path))
	{
		std::ofstream output(
			m_path.c_str(),
			std::ios::out | std::ios::binary
		);
		for (std::size_t i = 0; i < size; ++i)
		{
			output.put(static_cast<char>(i % 256));
		}
	}

	scoped_file(const scoped_file &other) = delete;
	scoped_file(scoped_file &&other) = delete;

	~scoped_file()
	{
		std::remove(m_path.c_str());
	}

	scoped_file& operator=(const scoped_file &other) = delete;
	scoped_file& operator=(scoped_file &&other) = delete;

	const std::string& get_path() const noexcept
	{
		return m_path;
	}

private:
	std::string m_path;
};

} // namespace

TEST_CASE( "image_probe reads the leading bytes once", "[image_probe]" )
{
	SECTION( "a file longer than the limit yields the whole of it" )
	{
		const scoped_file file(
			"test_image_probe_long.bin",
			image_probe::max_leading_bytes * 2
		);
		const image_probe probe(file.get_path());

		REQUIRE( probe.exists() );
		REQUIRE( probe.get_leading_bytes().size() ==
			image_probe::max_leading_bytes );
		REQUIRE( probe.get_leading_bytes()[0] == as_byte(0) );
		REQUIRE( probe.get_leading_bytes()[255] == as_byte(255) );
	}

	SECTION( "a file shorter than the limit yields what there is" )
	{
		const scoped_file file("test_image_probe_short.bin", 16);
		const image_probe probe(file.get_path());

		REQUIRE( probe.exists() );
		REQUIRE( probe.get_leading_bytes().size() == 16 );
	}

	SECTION( "an empty file yields no leading bytes" )
	{
		const scoped_file file("test_image_probe_empty.bin", 0);
		const image_probe probe(file.get_path());

		REQUIRE( probe.exists() );
		REQUIRE( probe.get_leading_bytes().empty() );
	}
}

TEST_CASE( "image_probe tolerates a file that is not there", "[image_probe]" )
{
	const image_probe probe("test_image_probe_absent.mrc");

	SECTION( "it reports the file as absent" )
	{
		REQUIRE_FALSE( probe.exists() );
	}

	SECTION( "it exposes no bytes to decide on" )
	{
		REQUIRE( probe.get_leading_bytes().empty() );
	}

	SECTION( "it still exposes the path and the extension" )
	{
		REQUIRE( probe.get_path() == "test_image_probe_absent.mrc" );
		REQUIRE( probe.get_extension() == ".mrc" );
	}
}

TEST_CASE( "image_probe normalizes the extension", "[image_probe]" )
{
	SECTION( "an extension is folded to lower case" )
	{
		const image_probe probe("absent.MRCS");

		REQUIRE( probe.get_extension() == ".mrcs" );
	}

	SECTION( "a mixed case extension is folded too" )
	{
		const image_probe probe("absent.TiFf");

		REQUIRE( probe.get_extension() == ".tiff" );
	}

	SECTION( "only the last extension is reported" )
	{
		const image_probe probe("absent.tar.gz");

		REQUIRE( probe.get_extension() == ".gz" );
	}

	SECTION( "a name without an extension yields nothing" )
	{
		const image_probe probe("absent");

		REQUIRE( probe.get_extension().empty() );
	}

	SECTION( "a dot in a directory is not an extension" )
	{
		const image_probe probe("a.b/absent");

		REQUIRE( probe.get_extension().empty() );
	}

	SECTION( "the path is kept exactly as it was given" )
	{
		const image_probe probe("Absent.MRCS");

		REQUIRE( probe.get_path() == "Absent.MRCS" );
	}
}
