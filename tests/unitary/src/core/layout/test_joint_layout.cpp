// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <rexlib/core/layout/joint_layout.hpp>

#include <core/layout/joint_layout_implementation.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;

TEST_CASE( "getting extents on an default constructed joint_layout should return empty ", "[joint_layout]" )
{
	joint_layout layout;
	const auto extents = layout.get_extents();
	REQUIRE( extents.empty() );
}

TEST_CASE( "getting extents on an default constructed joint_layout should have no operands ", "[joint_layout]" )
{
	joint_layout layout;
	REQUIRE( layout.get_number_of_operands() == 0 );
}

TEST_CASE( "getting strides on an default constructed joint_layout should throw ", "[joint_layout]" )
{
	joint_layout layout;
	REQUIRE_THROWS_AS( 
		layout.get_strides(0),
		std::out_of_range
	);
}

TEST_CASE( "getting the offset on an default constructed joint_layout should throw ", "[joint_layout]" )
{
	joint_layout layout;
	REQUIRE_THROWS_AS( 
		layout.get_offset(0),
		std::out_of_range
	);
};

TEST_CASE( "getting extents on an initialized joint_layout should return its extents ", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents =
		{ 20, 4, 16, 2 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	joint_layout layout(std::move(implementation));

	const auto result = layout.get_extents();
	REQUIRE( std::equal(extents.cbegin(), extents.cend(), result.begin(), result.end()) );
}

TEST_CASE( "computing the element count of a default constructed joint_layout should return zero", "[joint_layout]" )
{
	joint_layout layout;
	REQUIRE( layout.compute_element_count() == 0 );
}

TEST_CASE( "computing the element count of a joint_layout should return the product of its extents", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents =
		{ 20, 4, 16, 2 };
	auto implementation =
		std::make_unique<joint_layout_implementation>(extents);
	joint_layout layout(std::move(implementation));

	REQUIRE( layout.compute_element_count() == 20*4*16*2 );
}

TEST_CASE( "computing the element count of a joint_layout with no axes should return one", "[joint_layout]" )
{
	// The identity of the product: a layout of no axes holds a single
	// position, which is the one iter() reports and the one a traversal has
	// to visit.
	joint_layout_implementation::extent_vector_type extents;
	auto implementation =
		std::make_unique<joint_layout_implementation>(extents);
	joint_layout layout(std::move(implementation));

	REQUIRE( layout.compute_element_count() == 1 );
}

TEST_CASE( "computing the element count of a joint_layout with an empty axis should return zero", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = { 4, 0, 2 };
	auto implementation =
		std::make_unique<joint_layout_implementation>(extents);
	joint_layout layout(std::move(implementation));

	REQUIRE( layout.compute_element_count() == 0 );
}

TEST_CASE( "getting the number of operands on an joint_layout should return its operand count", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 0UL);
	implementation->add_operand(strides, 0UL);
	joint_layout layout(std::move(implementation));

	REQUIRE( layout.get_number_of_operands() == 2 );
}

TEST_CASE( "getting the strides of an operand on an joint_layout should return its strides", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 0UL);
	joint_layout layout(std::move(implementation));

	const auto result = layout.get_strides(0);
	REQUIRE( std::equal(strides.cbegin(), strides.cend(), result.begin(), result.end()) );
}

TEST_CASE( "getting the offset of an operand in an joint_layout should return its offset", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1234UL);
	joint_layout layout(std::move(implementation));

	REQUIRE( layout.get_offset(0) == 1234UL );
}

TEST_CASE( "getting the strides of an invalid operand on an joint_layout should throw", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 0UL);
	joint_layout layout(std::move(implementation));

	REQUIRE_THROWS_AS( 
		layout.get_strides(1),
		std::out_of_range
	);
}

TEST_CASE( "getting the offset of an invalid operand in an joint_layout should throw", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1234UL);
	joint_layout layout(std::move(implementation));

	REQUIRE_THROWS_AS( 
		layout.get_offset(1),
		std::out_of_range
	);
}

TEST_CASE( "calling iter on a default constructed array access layout should return 0", "[joint_layout]" )
{
	joint_layout layout;
	joint_cursor ite;

	CHECK( layout.iter(ite) == 0 );
}

TEST_CASE( "calling iter on an array access layout with an axis of size zero should return 0", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 0, 16, 2 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	CHECK( layout.iter(ite) == 0 );
}

TEST_CASE( "calling iter on an array access layout should populate the iterator and return the inner-most extent", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 20, 4, 16, 2 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 20, 80, 1280 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1234UL);
	implementation->add_operand(strides, 6532UL);
	implementation->add_operand(strides, 4UL);
	implementation->add_operand(strides, 8UL);
	implementation->add_operand(strides, 2UL);
	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	CHECK( layout.iter(ite) == extents.front() );
	
	const auto indices = ite.get_indices();
	CHECK( indices.size() == extents.size() );
	for (std::size_t i = 0; i < indices.size(); ++i)
	{
		CHECK( indices[i] == 0 );
	}

	const auto offsets = ite.get_offsets();
	REQUIRE( offsets.size() == 5 );
	CHECK( offsets[0] == 1234UL );
	CHECK( offsets[1] == 6532UL );
	CHECK( offsets[2] == 4UL );
	CHECK( offsets[3] == 8UL );
	CHECK( offsets[4] == 2UL );
}

TEST_CASE( "calling next on a default constructed array access layout should return 0", "[joint_layout]" )
{
	joint_layout layout;
	joint_cursor ite;

	CHECK( layout.next(ite, 1) == 0 );
}

TEST_CASE( "calling next on an array access layout on a stepping basis should advance indices and offsets", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 8, 2, 3 };
	joint_layout_implementation::stride_vector_type strides1 = 
		{ 1, 20, 80 };
	joint_layout_implementation::stride_vector_type strides2 = 
		{ 1, 8, 18 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides1, 2048UL);
	implementation->add_operand(strides2, 1024UL);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	REQUIRE( layout.iter(ite) == extents.front() );

	const auto offsets = ite.get_offsets();
	const auto indices = ite.get_indices();
	REQUIRE( offsets.size() == 2 );
	REQUIRE( indices.size() == 3 );

	for (std::size_t i = 0; i < extents[2]; ++i)
	{
		for (std::size_t j = 0; j < extents[1]; ++j)
		{
			for (std::size_t k = 0; k < extents[0]; ++k)
			{
				REQUIRE( indices[0] == k );
				REQUIRE( indices[1] == j );
				REQUIRE( indices[2] == i );
	
				CHECK( 2048 + k*strides1[0] + j*strides1[1] + i*strides1[2] == offsets[0] );
				CHECK( 1024 + k*strides2[0] + j*strides2[1] + i*strides2[2] == offsets[1] );

				std::size_t expected;
				if (k == extents[0]-1)
				{
					if (j == extents[1]-1 && i == extents[2]-1)
					{
						expected = 0;
					}
					else
					{
						expected = extents[0];
					}
				}
				else
				{
					expected = extents[0] - k - 1;
				}

				REQUIRE( layout.next(ite, 1) == expected );
			}
		}
	}

	CHECK( indices[0] == 0 );
	CHECK( indices[1] == 0 );
	CHECK( indices[2] == 0 );
	CHECK( offsets[0] == 2048 );
	CHECK( offsets[1] == 1024 );
}

TEST_CASE( "calling next on an array access layout on a block basis should advance indices and offsets", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 15, 2, 4 };
	joint_layout_implementation::stride_vector_type strides1 = 
		{ 1, 20, 80 };
	joint_layout_implementation::stride_vector_type strides2 = 
		{ 1, 15, 32 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides1, 2048UL);
	implementation->add_operand(strides2, 1024UL);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	REQUIRE( layout.iter(ite) == 15 );

	const auto offsets = ite.get_offsets();
	const auto indices = ite.get_indices();
	REQUIRE( offsets.size() == 2 );
	REQUIRE( indices.size() == 3 );

	for (std::size_t i = 0; i < extents[2]; ++i)
	{
		for (std::size_t j = 0; j < extents[1]; ++j)
		{
			REQUIRE( indices[0] == 0 );
			REQUIRE( indices[1] == j );
			REQUIRE( indices[2] == i );
			CHECK( 2048 + j*strides1[1] + i*strides1[2] == offsets[0] );
			CHECK( 1024 + j*strides2[1] + i*strides2[2] == offsets[1] );

			REQUIRE( layout.next(ite, 8) == 7 );

			REQUIRE( indices[0] == 8 );
			REQUIRE( indices[1] == j );
			REQUIRE( indices[2] == i );
			CHECK( 2048 + 8*strides1[0] + j*strides1[1] + i*strides1[2] == offsets[0] );
			CHECK( 1024 + 8*strides2[0] + j*strides2[1] + i*strides2[2] == offsets[1] );

			std::size_t expected;
			if (j == extents[1]-1 && i == extents[2]-1)
			{
				expected = 0;
			}
			else
			{
				expected = 15;
			}

			REQUIRE( layout.next(ite, 7) == expected );
		}
	}

	CHECK( indices[0] == 0 );
	CHECK( indices[1] == 0 );
	CHECK( indices[2] == 0 );
	CHECK( offsets[0] == 2048 );
	CHECK( offsets[1] == 1024 );
}

TEST_CASE( "calling next on an array access layout on an first_dim should posterior indices and offsets", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides1 = 
		{ 1, 5, 80 };
	joint_layout_implementation::stride_vector_type strides2 = 
		{ 1, 4, 60 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides1, 2048UL);
	implementation->add_operand(strides2, 1024UL);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	REQUIRE( layout.iter(ite, 1) == 15 );

	const auto offsets = ite.get_offsets();
	const auto indices = ite.get_indices();
	REQUIRE( offsets.size() == 2 );
	REQUIRE( indices.size() == 3 );

	const auto first_index = GENERATE(0, 2, 3);
	indices[0] = first_index; // Should not be modified.

	for (std::size_t i = 0; i < extents[2]; ++i)
	{
		REQUIRE( indices[0] == first_index );
		REQUIRE( indices[1] == 0 );
		REQUIRE( indices[2] == i );
		REQUIRE( 2048 + i*strides1[2] == offsets[0] );
		REQUIRE( 1024 + i*strides2[2] == offsets[1] );

		REQUIRE( layout.next(ite, 8, 1) == 7 );

		REQUIRE( indices[0] == first_index );
		REQUIRE( indices[1] == 8 );
		REQUIRE( indices[2] == i );
		REQUIRE( 2048 + 8*strides1[1] + i*strides1[2] == offsets[0] );
		REQUIRE( 1024 + 8*strides2[1] + i*strides2[2] == offsets[1] );

		std::size_t expected;
		if (i == extents[2]-1)
		{
			expected = 0;
		}
		else
		{
			expected = 15;
		}

		REQUIRE( layout.next(ite, 7, 1) == expected );
	}

	CHECK( indices[0] == first_index );
	CHECK( indices[1] == 0 );
	CHECK( indices[2] == 0 );
	CHECK( offsets[0] == 2048 );
	CHECK( offsets[1] == 1024 );
}

TEST_CASE( "calling next on an array access layout with first_dim and last_dim should update in-between indices and offsets", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides1 = 
		{ 1, 5, 80 };
	joint_layout_implementation::stride_vector_type strides2 = 
		{ 1, 4, 60 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides1, 2048UL);
	implementation->add_operand(strides2, 1024UL);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	REQUIRE( layout.iter(ite, 1, 2) == 15 );

	const auto offsets = ite.get_offsets();
	const auto indices = ite.get_indices();
	REQUIRE( offsets.size() == 2 );
	REQUIRE( indices.size() == 3 );

	const auto first_index = GENERATE(0, 2, 3);
	const auto last_index = GENERATE(0, 2, 3);
	indices[0] = first_index; // Should not be modified.
	indices[2] = last_index; // Should not be modified.

	REQUIRE( indices[1] == 0 );
	REQUIRE( offsets[0] == 2048);
	REQUIRE( offsets[1] == 1024);

	REQUIRE( layout.next(ite, 8, 1, 2) == 7 );

	REQUIRE( indices[0] == first_index );
	REQUIRE( indices[1] == 8 );
	REQUIRE( indices[2] == last_index );
	REQUIRE( 2048 + 8*strides1[1] == offsets[0] );
	REQUIRE( 1024 + 8*strides2[1] == offsets[1] );

	REQUIRE( layout.next(ite, 7, 1, 2) == 0 );
}

TEST_CASE( "calling next on an array access layout with same first_dim and last_dim should prompt single iteration", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides1 = 
		{ 1, 5, 80 };
	joint_layout_implementation::stride_vector_type strides2 = 
		{ 1, 4, 60 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides1, 2048UL);
	implementation->add_operand(strides2, 1024UL);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	REQUIRE( layout.iter(ite, 2, 2) == 1 );
	REQUIRE( layout.next(ite, 1, 2, 2) == 0 );
}

TEST_CASE( "calling next on an array access layout with zero step should not advance the iterator", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 5, 80 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1024);

	joint_layout layout(std::move(implementation));
	joint_cursor ite;

	const auto dim = GENERATE(0, 1, 2);
	REQUIRE( layout.iter(ite, dim) == extents[dim] );

	const auto expected_ite = ite;
	const auto expected_offsets = expected_ite.get_offsets();
	const auto expected_indices = expected_ite.get_indices();
	for (std::size_t i = 0; i < 100; ++i)
	{
		REQUIRE( layout.next(ite, 0, dim) == extents[dim] );

		const auto offsets = ite.get_offsets();
		const auto indices = ite.get_indices();
		REQUIRE( std::equal(expected_offsets.begin(), expected_offsets.end(), offsets.begin(), offsets.end()) );
		REQUIRE( std::equal(expected_indices.begin(), expected_indices.end(), indices.begin(), indices.end()) );
	}

}

TEST_CASE( "calling iter on a joint_layout with dim equal to rank should return one", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 5, 80 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1024);

	joint_layout layout(std::move(implementation));

	joint_cursor ite;
	REQUIRE( layout.iter(ite, 3) == 1 );
}

TEST_CASE( "calling next on a joint_layout with dim equal to rank should return zero", "[joint_layout]" )
{
	joint_layout_implementation::extent_vector_type extents = 
		{ 4, 15, 4 };
	joint_layout_implementation::stride_vector_type strides = 
		{ 1, 5, 80 };
	auto implementation = 
		std::make_unique<joint_layout_implementation>(extents);
	implementation->add_operand(strides, 1024);

	joint_layout layout(std::move(implementation));

	joint_cursor ite;
	REQUIRE( layout.iter(ite, 3) == 1 );
	REQUIRE( layout.next(ite, 1, 3) == 0 );
}


namespace
{

/**
 * @brief Build a layout over the given extents with one operand per stride
 * set, each starting at a distinct offset.
 */
joint_layout make_seek_layout(
	const joint_layout_implementation::extent_vector_type &extents,
	const std::vector<
		joint_layout_implementation::stride_vector_type
	> &operand_strides
)
{
	auto implementation =
		std::make_unique<joint_layout_implementation>(extents);
	for (std::size_t i = 0; i < operand_strides.size(); ++i)
	{
		implementation->add_operand(
			operand_strides[i],
			static_cast<std::ptrdiff_t>(100*(i + 1))
		);
	}

	return joint_layout(std::move(implementation));
}

/**
 * @brief Where a cursor sits, as the pair of things a caller can read off it.
 */
struct cursor_state
{
	std::vector<std::size_t> indices;
	std::vector<std::ptrdiff_t> offsets;

	bool operator==(const cursor_state &other) const
	{
		return indices == other.indices && offsets == other.offsets;
	}
};

cursor_state read_cursor(const joint_cursor &ite)
{
	const auto indices = ite.get_indices();
	const auto offsets = ite.get_offsets();

	return cursor_state {
		std::vector<std::size_t>(indices.begin(), indices.end()),
		std::vector<std::ptrdiff_t>(offsets.begin(), offsets.end())
	};
}

} // anonymous namespace

TEST_CASE(
	"seek on a joint_layout should land where stepping there lands",
	"[joint_layout]"
)
{
	// Brute force over every position of the iteration space, against the
	// only other way of reaching it: iter() followed by that many single
	// steps of next(). The two have to agree on the indices and on every
	// operand's offset, because that agreement is exactly what lets an
	// iteration be cut into ranges and shared out.
	joint_layout_implementation::extent_vector_type extents;
	std::vector<joint_layout_implementation::stride_vector_type> strides;

	SECTION( "a contiguous layout" )
	{
		extents = { 4, 3, 2 };
		strides = { { 1, 4, 12 } };
	}
	SECTION( "several operands walked differently" )
	{
		extents = { 4, 3, 2 };
		strides = { { 1, 4, 12 }, { 12, 4, 1 }, { 2, 8, 24 } };
	}
	SECTION( "an operand broadcast along an axis" )
	{
		extents = { 4, 3, 2 };
		strides = { { 1, 4, 12 }, { 1, 0, 4 } };
	}
	SECTION( "an operand walked backwards" )
	{
		extents = { 4, 3, 2 };
		strides = { { -1, -4, -12 } };
	}
	SECTION( "an axis of extent one" )
	{
		extents = { 4, 1, 2 };
		strides = { { 1, 4, 4 } };
	}
	SECTION( "a single axis" )
	{
		extents = { 7 };
		strides = { { 3 } };
	}
	SECTION( "a deeper layout" )
	{
		extents = { 2, 3, 2, 2 };
		strides = { { 1, 2, 6, 12 }, { 24, 8, 4, 1 } };
	}

	const auto layout = make_seek_layout(extents, strides);

	std::size_t count = 1;
	for (const auto extent : extents)
	{
		count *= extent;
	}

	joint_cursor stepped;
	REQUIRE( layout.iter(stepped) != 0 );

	for (std::size_t position = 0; position < count; ++position)
	{
		joint_cursor sought;
		const auto run = layout.seek(sought, position);

		INFO( "position " << position );
		CHECK( run == extents.front() - (position % extents.front()) );
		CHECK( read_cursor(sought) == read_cursor(stepped) );

		layout.next(stepped, 1);
	}
}

TEST_CASE(
	"seek on a joint_layout with a position of zero should behave as iter",
	"[joint_layout]"
)
{
	joint_layout_implementation::extent_vector_type extents = { 4, 3, 2 };
	const auto layout = make_seek_layout(extents, { { 1, 4, 12 }, { 12, 4, 1 } });

	joint_cursor sought;
	joint_cursor started;

	CHECK( layout.seek(sought, 0) == layout.iter(started) );
	CHECK( read_cursor(sought) == read_cursor(started) );
}

TEST_CASE(
	"seek on a joint_layout should count only the iterated axes",
	"[joint_layout]"
)
{
	// With the inner-most axis left out of the window, the position counts
	// over the remaining axes alone, and the axes outside it stay put.
	joint_layout_implementation::extent_vector_type extents = { 4, 3, 2 };
	const auto layout = make_seek_layout(extents, { { 1, 4, 12 } });

	joint_cursor stepped;
	REQUIRE( layout.iter(stepped, 1) == 3 );

	for (std::size_t position = 0; position < 3*2; ++position)
	{
		joint_cursor sought;
		const auto run = layout.seek(sought, position, 1);

		INFO( "position " << position );
		CHECK( run == 3 - (position % 3) );
		CHECK( read_cursor(sought) == read_cursor(stepped) );

		layout.next(stepped, 1, 1);
	}
}

TEST_CASE(
	"seek on a default constructed joint_layout should return zero",
	"[joint_layout]"
)
{
	joint_layout layout;
	joint_cursor ite;

	CHECK( layout.seek(ite, 0) == 0 );
}

TEST_CASE(
	"seek on a joint_layout with an axis of size zero should return zero",
	"[joint_layout]"
)
{
	joint_layout_implementation::extent_vector_type extents = { 4, 0, 2 };
	const auto layout = make_seek_layout(extents, { { 1, 4, 4 } });

	joint_cursor ite;
	CHECK( layout.seek(ite, 0) == 0 );
}

TEST_CASE(
	"seek on a joint_layout with dim equal to rank should return one",
	"[joint_layout]"
)
{
	// A window holding a single position: there is nowhere to seek to, and
	// the answer is the one iter() gives.
	joint_layout_implementation::extent_vector_type extents = { 4, 15, 4 };
	const auto layout = make_seek_layout(extents, { { 1, 5, 80 } });

	joint_cursor ite;
	CHECK( layout.seek(ite, 0, 3) == 1 );
}
