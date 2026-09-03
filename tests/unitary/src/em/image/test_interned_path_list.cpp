// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/interned_path_list.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>

using namespace rexlib;
using namespace rexlib::em;

TEST_CASE( "an interned_path_list starts empty", "[interned_path_list]" )
{
	const interned_path_list files;

	REQUIRE( files.get_size() == 0 );
	REQUIRE( files.get_path_count() == 0 );
	REQUIRE( files.find("stack_0.mrcs") == interned_path_list::no_path );
}

TEST_CASE( "interning gives equal paths one index", "[interned_path_list]" )
{
	interned_path_list files;

	SECTION( "a path interned twice keeps the index it was first given" )
	{
		const auto first = files.intern("stack_0.mrcs");
		const auto again = files.intern("stack_0.mrcs");

		REQUIRE( first == again );
		REQUIRE( files.get_path_count() == 1 );
	}

	SECTION( "distinct paths get distinct indices" )
	{
		const auto first = files.intern("stack_0.mrcs");
		const auto second = files.intern("stack_1.mrcs");

		REQUIRE( first != second );
		REQUIRE( files.get_path_count() == 2 );
		REQUIRE( files.get_path(first) == "stack_0.mrcs" );
		REQUIRE( files.get_path(second) == "stack_1.mrcs" );
	}

	SECTION( "interning appends no entry" )
	{
		files.intern("stack_0.mrcs");

		REQUIRE( files.get_size() == 0 );
	}

	SECTION( "find reports the index a path was interned under" )
	{
		const auto index = files.intern("stack_0.mrcs");

		REQUIRE( files.find("stack_0.mrcs") == index );
		REQUIRE( files.find("stack_1.mrcs") == interned_path_list::no_path );
	}
}

TEST_CASE( "an interned_path_list holds more entries than paths",
	"[interned_path_list]" )
{
	// The shape the type exists for: many regions naming few files.
	interned_path_list files;
	const auto first = files.intern("stack_0.mrcs");
	const auto second = files.intern("stack_1.mrcs");

	files.append(first);
	files.append(second);
	files.append(first);

	SECTION( "the two sizes are counted separately" )
	{
		REQUIRE( files.get_path_count() == 2 );
		REQUIRE( files.get_size() == 3 );
	}

	SECTION( "each entry reports the path it refers to" )
	{
		REQUIRE( files.get(0) == "stack_0.mrcs" );
		REQUIRE( files.get(1) == "stack_1.mrcs" );
		REQUIRE( files.get(2) == "stack_0.mrcs" );
	}

	SECTION( "each entry reports which interned path it refers to" )
	{
		REQUIRE( files.get_path_index(0) == first );
		REQUIRE( files.get_path_index(1) == second );
		REQUIRE( files.get_path_index(2) == first );
	}

	SECTION( "entries naming one path share its string" )
	{
		REQUIRE( &files.get(0) == &files.get(2) );
	}
}

TEST_CASE( "appending a path interns it as well", "[interned_path_list]" )
{
	interned_path_list files;

	SECTION( "the returned position is the entry, not the path" )
	{
		REQUIRE( files.append(std::string("stack_0.mrcs")) == 0 );
		REQUIRE( files.append(std::string("stack_0.mrcs")) == 1 );
		REQUIRE( files.get_path_count() == 1 );
		REQUIRE( files.get_size() == 2 );
	}

	SECTION( "an entry naming no interned path is refused" )
	{
		files.intern("stack_0.mrcs");

		REQUIRE_THROWS_AS( files.append(std::size_t(1)), std::out_of_range );
		REQUIRE( files.get_size() == 0 );
	}
}

TEST_CASE( "clearing an interned_path_list drops paths and entries",
	"[interned_path_list]" )
{
	interned_path_list files;
	files.append(std::string("stack_0.mrcs"));
	files.append(std::string("stack_1.mrcs"));
	files.clear();

	REQUIRE( files.get_size() == 0 );
	REQUIRE( files.get_path_count() == 0 );
	REQUIRE( files.find("stack_0.mrcs") == interned_path_list::no_path );
}

TEST_CASE( "an interned_path_list reused across calls stops allocating",
	"[interned_path_list]" )
{
	const std::size_t count = 64;

	interned_path_list files;
	files.reserve(2, count);

	const auto fill = [&] ()
	{
		files.clear();
		const auto first = files.intern("stack_0.mrcs");
		const auto second = files.intern("stack_1.mrcs");
		for (std::size_t i = 0; i < count; ++i)
		{
			files.append((i % 2) == 0 ? first : second);
		}
	};

	fill();
	const auto *paths = &files.get_path(0);

	for (std::size_t repeat = 0; repeat < 8; ++repeat)
	{
		fill();

		REQUIRE( files.get_size() == count );
		REQUIRE( files.get_path_count() == 2 );
		REQUIRE( files.get(count - 1) == "stack_1.mrcs" );
		REQUIRE( &files.get_path(0) == paths );
	}
}

TEST_CASE( "an interned_path_list has value semantics",
	"[interned_path_list]" )
{
	interned_path_list files;
	files.append(std::string("stack_0.mrcs"));

	SECTION( "a copy holds the same paths and entries" )
	{
		const interned_path_list copy(files);

		REQUIRE( copy.get_size() == 1 );
		REQUIRE( copy.get_path_count() == 1 );
		REQUIRE( copy.get(0) == "stack_0.mrcs" );
	}

	SECTION( "a copy does not share storage with the list it copies" )
	{
		interned_path_list copy(files);
		copy.clear();

		REQUIRE( copy.get_size() == 0 );
		REQUIRE( files.get_size() == 1 );
	}
}
