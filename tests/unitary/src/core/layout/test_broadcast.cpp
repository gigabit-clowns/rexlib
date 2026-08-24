// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/layout/broadcast.hpp>

#include <rex/core/layout/broadcast_error.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <cstddef>
#include <vector>

using namespace rexlib;


TEST_CASE(
	"broadcast_extents with compatible extents should succeed",
	"[broadcast]"
)
{
	std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	std::vector<std::size_t> extents2 = {5, 6, 1, 3};

	broadcast_extents(extents1, extents2);

	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( extents1 == expected );
	CHECK( extents2 == expected );
}

TEST_CASE(
	"broadcast_extents with missing trailing axis in the right should succeed",
	"[broadcast]"
)
{
	std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	std::vector<std::size_t> extents2 = {6, 1, 3};

	broadcast_extents(extents1, extents2);

	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( extents1 == expected );
	CHECK( extents2 == expected );
}

TEST_CASE(
	"broadcast_extents with missing trailing axis in the left should succeed",
	"[broadcast]"
)
{
	std::vector<std::size_t> extents1 = {1, 4, 3};
	std::vector<std::size_t> extents2 = {5, 6, 1, 3};

	broadcast_extents(extents1, extents2);

	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( extents1 == expected );
	CHECK( extents2 == expected );
}

TEST_CASE(
	"broadcast_extents with incompatible axis should fail",
	"[broadcast]"
)
{
	std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	std::vector<std::size_t> extents2 = {5, 6, 2, 3};

	REQUIRE_THROWS_MATCHES(
		broadcast_extents(extents1, extents2),
		broadcast_error,
		Catch::Matchers::Message(
			"Broadcast error: extents (5, 6, 4, 3) and (5, 6, 2, 3) "
			"are not compatible."
		)
	);
}


TEST_CASE(
	"broadcast_extents_accumulate with equal-rank compatible extents stores "
	"the broadcast result",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 1, 4, 3};
	const std::vector<std::size_t> other = {5, 6, 1, 3};

	broadcast_extents_accumulate(result, make_span(other));

	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate fills unit axes of the result from other",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {1, 1, 4};
	const std::vector<std::size_t> other = {5, 6, 4};

	broadcast_extents_accumulate(result, make_span(other));

	const std::vector<std::size_t> expected = {5, 6, 4};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate keeps non-unit axes when other has unit axes",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 6, 4};
	const std::vector<std::size_t> other = {1, 1, 1};

	broadcast_extents_accumulate(result, make_span(other));

	// The non-unit extents already held by result are preserved.
	const std::vector<std::size_t> expected = {5, 6, 4};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate pads a lower-rank result with leading axes",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {4, 3};
	const std::vector<std::size_t> other = {5, 6, 4, 3};

	broadcast_extents_accumulate(result, make_span(other));

	// result is padded with leading ones up to the rank of other and then
	// broadcasted element-wise.
	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate with a lower-rank other leaves the leading "
	"axes of result untouched",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 6, 4, 3};
	const std::vector<std::size_t> other = {1, 3};

	broadcast_extents_accumulate(result, make_span(other));

	// other is implicitly padded with leading ones, so the higher-rank leading
	// axes of result are kept and only its trailing axes are broadcasted.
	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate with a lower-rank other broadcasts into the "
	"trailing axes",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 6, 1, 3};
	const std::vector<std::size_t> other = {4, 3};

	broadcast_extents_accumulate(result, make_span(other));

	const std::vector<std::size_t> expected = {5, 6, 4, 3};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate with a lower-rank other and incompatible "
	"trailing axes should fail",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 6, 4, 3};
	const std::vector<std::size_t> other = {2, 3};

	REQUIRE_THROWS_MATCHES(
		broadcast_extents_accumulate(result, make_span(other)),
		broadcast_error,
		Catch::Matchers::Message(
			"Broadcast error: extents (5, 6, 4, 3) and (2, 3) "
			"are not compatible."
		)
	);
}

TEST_CASE(
	"broadcast_extents_accumulate into an empty result takes the shape of other",
	"[broadcast]"
)
{
	std::vector<std::size_t> result;
	const std::vector<std::size_t> other = {5, 6, 4};

	broadcast_extents_accumulate(result, make_span(other));

	const std::vector<std::size_t> expected = {5, 6, 4};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate with two empty extents leaves the result "
	"empty",
	"[broadcast]"
)
{
	std::vector<std::size_t> result;
	const std::vector<std::size_t> other;

	broadcast_extents_accumulate(result, make_span(other));

	CHECK( result.empty() );
}

TEST_CASE(
	"broadcast_extents_accumulate accumulates the broadcast shape across "
	"repeated calls",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {3};
	const std::vector<std::size_t> other1 = {4, 1};
	const std::vector<std::size_t> other2 = {5, 1, 1};

	broadcast_extents_accumulate(result, make_span(other1));
	broadcast_extents_accumulate(result, make_span(other2));

	// Each call merges the next set of extents into the running result.
	const std::vector<std::size_t> expected = {5, 4, 3};
	CHECK( result == expected );
}

TEST_CASE(
	"broadcast_extents_accumulate with incompatible extents should fail",
	"[broadcast]"
)
{
	std::vector<std::size_t> result = {5, 1, 4, 3};
	const std::vector<std::size_t> other = {5, 6, 2, 3};

	REQUIRE_THROWS_MATCHES(
		broadcast_extents_accumulate(result, make_span(other)),
		broadcast_error,
		Catch::Matchers::Message(
			"Broadcast error: extents (5, 6, 4, 3) and (5, 6, 2, 3) "
			"are not compatible."
		)
	);
}


TEST_CASE(
	"is_broadcast_compatible with equal extents should return true",
	"[broadcast]"
)
{
	const std::vector<std::size_t> extents1 = {5, 6, 4, 3};
	const std::vector<std::size_t> extents2 = {5, 6, 4, 3};

	CHECK( is_broadcast_compatible(make_span(extents1), make_span(extents2)) );
}

TEST_CASE(
	"is_broadcast_compatible with broadcast-compatible extents should return "
	"true",
	"[broadcast]"
)
{
	const std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	const std::vector<std::size_t> extents2 = {5, 6, 1, 3};

	CHECK( is_broadcast_compatible(make_span(extents1), make_span(extents2)) );
}

TEST_CASE(
	"is_broadcast_compatible with different rank extents should return true "
	"when compatible",
	"[broadcast]"
)
{
	const std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	const std::vector<std::size_t> extents2 = {6, 1, 3};

	CHECK( is_broadcast_compatible(make_span(extents1), make_span(extents2)) );
}

TEST_CASE(
	"is_broadcast_compatible with incompatible extents should return false",
	"[broadcast]"
)
{
	const std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	const std::vector<std::size_t> extents2 = {5, 6, 2, 3};

	CHECK_FALSE(
		is_broadcast_compatible(make_span(extents1), make_span(extents2))
	);
}

TEST_CASE(
	"is_broadcast_compatible is symmetric",
	"[broadcast]"
)
{
	const std::vector<std::size_t> extents1 = {5, 1, 4, 3};
	const std::vector<std::size_t> extents2 = {5, 6, 1, 3};

	CHECK(
		is_broadcast_compatible(make_span(extents1), make_span(extents2)) ==
		is_broadcast_compatible(make_span(extents2), make_span(extents1))
	);
}


TEST_CASE(
	"is_broadcastable_to with equal extents should return true",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {5, 6, 4, 3};
	const std::vector<std::size_t> to_extents   = {5, 6, 4, 3};

	CHECK(
		is_broadcastable_to(make_span(from_extents), make_span(to_extents))
	);
}

TEST_CASE(
	"is_broadcastable_to with ones in source should return true",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {5, 1, 4, 1};
	const std::vector<std::size_t> to_extents   = {5, 6, 4, 3};

	CHECK(
		is_broadcastable_to(make_span(from_extents), make_span(to_extents))
	);
}

TEST_CASE(
	"is_broadcastable_to with smaller rank source should return true when "
	"compatible",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {1, 4, 3};
	const std::vector<std::size_t> to_extents   = {5, 6, 4, 3};

	CHECK(
		is_broadcastable_to(make_span(from_extents), make_span(to_extents))
	);
}

TEST_CASE(
	"is_broadcastable_to with larger rank source should return false",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {5, 6, 4, 3};
	const std::vector<std::size_t> to_extents   = {6, 4, 3};

	CHECK_FALSE(
		is_broadcastable_to(make_span(from_extents), make_span(to_extents))
	);
}

TEST_CASE(
	"is_broadcastable_to with mismatched non-unit extent should return false",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {5, 2, 4, 3};
	const std::vector<std::size_t> to_extents   = {5, 6, 4, 3};

	CHECK_FALSE(
		is_broadcastable_to(make_span(from_extents), make_span(to_extents))
	);
}

TEST_CASE(
	"is_broadcastable_to is not symmetric when source has non-unit extent "
	"matching target",
	"[broadcast]"
)
{
	const std::vector<std::size_t> from_extents = {1, 4, 3};
	const std::vector<std::size_t> to_extents   = {5, 4, 3};

	CHECK( is_broadcastable_to(make_span(from_extents), make_span(to_extents)) );
	CHECK_FALSE(
		is_broadcastable_to(make_span(to_extents), make_span(from_extents))
	);
}
