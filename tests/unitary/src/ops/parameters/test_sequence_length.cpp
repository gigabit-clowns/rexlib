// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/parameters/sequence_length.hpp>

using namespace rexlib::ops;

TEST_CASE(
	"sequence_length should store its count",
	"[sequence_length]"
)
{
	CHECK( sequence_length(5).get_count() == 5 );
	CHECK( sequence_length(0).get_count() == 0 );
}
