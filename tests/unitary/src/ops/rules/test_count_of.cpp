// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/rules/count_of.hpp>

#include <rexlib/ops/parameters/sequence_length.hpp>

using namespace rexlib::ops;

TEST_CASE(
	"count_of should read the count of the parameters",
	"[count_of]"
)
{
	CHECK( count_of::get(sequence_length(7)) == 7 );
}
