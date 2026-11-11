// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/ops/parameters/signal_extent.hpp>

using namespace rexlib;
using namespace rexlib::ops;

TEST_CASE(
	"signal_extent should store its extent",
	"[signal_extent]"
)
{
	const signal_extent parameters(17);
	CHECK( parameters.get_extent() == 17 );
}
