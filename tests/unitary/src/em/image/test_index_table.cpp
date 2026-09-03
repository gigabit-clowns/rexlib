// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/image/index_table.hpp>

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;

namespace
{

std::vector<std::size_t> to_vector(span<const std::size_t> values)
{
	return std::vector<std::size_t>(values.begin(), values.end());
}

} // namespace

TEST_CASE( "an index_table starts empty", "[index_table]" )
{
	SECTION( "the default table has rank zero" )
	{
		const index_table table;

		REQUIRE( table.get_rank() == 0 );
		REQUIRE( table.get_size() == 0 );
	}

	SECTION( "a table constructed with a rank carries it" )
	{
		const index_table table(3);

		REQUIRE( table.get_rank() == 3 );
		REQUIRE( table.get_size() == 0 );
	}
}

TEST_CASE( "an index_table reads back what it was given", "[index_table]" )
{
	index_table table(3);

	SECTION( "indices are read back in order" )
	{
		const std::size_t first[3] = {2, 0, 0};
		const std::size_t second[3] = {1, 4, 7};
		table.add(make_span(first, 3));
		table.add(make_span(second, 3));

		REQUIRE( table.get_size() == 2 );
		REQUIRE( to_vector(table.get(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
		REQUIRE( to_vector(table.get(1)) ==
			std::vector<std::size_t>{1, 4, 7} );
	}

	SECTION( "clear drops the indices but keeps the rank" )
	{
		const std::size_t index[3] = {0, 0, 0};
		table.add(make_span(index, 3));
		table.clear();

		REQUIRE( table.get_size() == 0 );
		REQUIRE( table.get_rank() == 3 );
	}
}

TEST_CASE( "an index_table refuses an index of the wrong rank",
	"[index_table]" )
{
	index_table table(3);
	const std::size_t two[2] = {0, 0};

	SECTION( "an index of the wrong rank throws" )
	{
		REQUIRE_THROWS_AS(
			table.add(make_span(two, 2)),
			std::invalid_argument
		);
	}

	SECTION( "a refused index is not appended" )
	{
		REQUIRE_THROWS_AS(
			table.add(make_span(two, 2)),
			std::invalid_argument
		);
		REQUIRE( table.get_size() == 0 );
	}
}

TEST_CASE( "an index_table of rank zero holds indices addressing nothing",
	"[index_table]" )
{
	// A plan over a side of rank zero still has one index per region, and
	// each of them carries no coordinate at all.
	index_table table(0);
	table.add(span<const std::size_t>());
	table.add(span<const std::size_t>());

	REQUIRE( table.get_size() == 2 );
	REQUIRE( table.get(0).empty() );
	REQUIRE( table.get(1).empty() );
}

TEST_CASE( "an index_table reused across calls stops allocating",
	"[index_table]" )
{
	// This is the reason the indices are held flat rather than one vector
	// each: once the first fill has sized it, filling it again touches the
	// heap not at all.
	const std::size_t count = 64;

	index_table table(3);
	table.reserve(count);

	const auto fill = [&] ()
	{
		table.clear();
		for (std::size_t i = 0; i < count; ++i)
		{
			const std::size_t index[3] = {i, 0, 0};
			table.add(make_span(index, 3));
		}
	};

	fill();
	const auto *data = table.get(0).data();

	SECTION( "refilling keeps every index" )
	{
		fill();

		REQUIRE( table.get_size() == count );
		REQUIRE( to_vector(table.get(count - 1)) ==
			std::vector<std::size_t>{count - 1, 0, 0} );
	}

	SECTION( "refilling does not move the storage, so nothing is allocated" )
	{
		for (std::size_t repeat = 0; repeat < 8; ++repeat)
		{
			fill();
			REQUIRE( table.get(0).data() == data );
		}
	}
}

TEST_CASE( "an index_table has value semantics", "[index_table]" )
{
	index_table table(3);
	const std::size_t index[3] = {2, 0, 0};
	table.add(make_span(index, 3));

	SECTION( "a copy holds the same indices" )
	{
		const index_table copy(table);

		REQUIRE( copy.get_rank() == 3 );
		REQUIRE( copy.get_size() == 1 );
		REQUIRE( to_vector(copy.get(0)) ==
			std::vector<std::size_t>{2, 0, 0} );
	}

	SECTION( "a copy does not share storage with the table it copies" )
	{
		index_table copy(table);
		copy.clear();

		REQUIRE( copy.get_size() == 0 );
		REQUIRE( table.get_size() == 1 );
	}
}
