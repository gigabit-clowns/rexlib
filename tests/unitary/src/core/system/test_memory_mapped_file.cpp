// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <xmipp4/core/system/memory_mapped_file.hpp>

#include <xmipp4/core/system/access_flags.hpp>

#include <fstream>
#include <string>
#include <cstring>

using namespace xmipp4;

static void write_test_pattern1(const std::string &path, std::size_t n)
{
	std::string data(n, 0);
	for (std::size_t i = 0; i < data.size(); ++i)
	{
		data[i] = static_cast<char>('a' + (i%32));
	}

	std::ofstream output_file(path.c_str());
	output_file.write(data.c_str(), data.size());
}

static bool check_test_pattern1(const char* data, std::size_t count)
{
	for(std::size_t i = 0; i < count; ++i)
	{
		if(data[i] != static_cast<char>('a' + (i%32)))
			return false;
	}
	return true;
}

static void fill_test_pattern2(char* data, std::size_t count)
{
	for(std::size_t i = 0; i < count; ++i)
	{
		data[i] = static_cast<char>('0' + (i%10));
	}
}

static bool check_test_pattern2(const char* data, std::size_t count)
{
	for(std::size_t i = 0; i < count; ++i)
	{
		if(data[i] != static_cast<char>('0' + (i%10)))
			return false;
	}
	return true;
}

TEST_CASE( "memory map a file for reading and writing", "[memory_mapped_file]" ) 
{
	const std::string path = "memory_mapped_file.txt";
	REX_CONST_CONSTEXPR std::size_t n = 1 << 20; // 1 MiB
	write_test_pattern1(path, n);

	SECTION ("Read first few bytes")
	{
		memory_mapped_file mm(
			path, 
			read_only, 
			8,
			false
		);

		// Expect the first 8 bytes to be equal to the test data
		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == 8 );
		REQUIRE( check_test_pattern1(static_cast<const char*>(mm.data()), mm.size()) );
	}

	SECTION ("Read the entire file")
	{
		memory_mapped_file mm(
			path, 
			read_only, 
			memory_mapped_file::whole_file,
			false
		);
		
		// Expect the whole range to be equal to the test data
		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == n );
		REQUIRE( check_test_pattern1(static_cast<const char*>(mm.data()), mm.size()) );
	}

	SECTION ("Write the entire file as copy on write")
	{
		memory_mapped_file mm(
			path, 
			read_write, 
			memory_mapped_file::whole_file,
			true
		);

		// Check that the test data has been read
		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == n );
		REQUIRE( check_test_pattern1(static_cast<const char*>(mm.data()), mm.size()) );

		// Write some random data and close
		fill_test_pattern2(static_cast<char*>(mm.data()), mm.size());
		mm.close();
		REQUIRE( !mm.is_open() );

		// Re-open and expect the original test data
		mm.open(
			path, 
			read_only, 
			memory_mapped_file::whole_file,
			false
		);

		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == n );
		REQUIRE( check_test_pattern1(static_cast<const char*>(mm.data()), mm.size()) );
	}

	SECTION ("Write the entire file")
	{
		memory_mapped_file mm(
			path, 
			read_write, 
			memory_mapped_file::whole_file,
			false
		);

		// Check that the test data has been read
		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == n );
		REQUIRE( check_test_pattern1(static_cast<const char*>(mm.data()), mm.size()) );

		// Write some random data and close
		fill_test_pattern2(static_cast<char*>(mm.data()), mm.size());
		mm.close();
		REQUIRE( !mm.is_open() );

		// Re-open and expect the modified test data
		mm.open(
			path, 
			read_only, 
			memory_mapped_file::whole_file,
			false
		);

		REQUIRE( mm.is_open() );
		REQUIRE( mm.size() == n );
		REQUIRE( check_test_pattern2(static_cast<const char*>(mm.data()), mm.size()) );
	}
}
