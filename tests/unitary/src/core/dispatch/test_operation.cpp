// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/operation.hpp>

#include "mock/mock_operation.hpp"

using namespace rexlib;

TEST_CASE("Operations should return the correct operation_id", "[operation]")
{
	mock_operation op;
	CHECK( op.get_id() == operation_id::of<mock_operation>() );
}
