// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/operation_id.hpp>

#include "mock/mock_operation.hpp"

using namespace rex;

TEST_CASE("Constructing an operation_id should store its data", "[operation_id]")
{
	const operation_id id(typeid(mock_operation));

	CHECK( id.hash() == std::hash<std::type_index>()(typeid(mock_operation)) );
}

TEST_CASE("The factory method of operation_id should call its constructor", "[operation_id]")
{
	const auto id1 = operation_id::of<mock_operation>();
	const operation_id id2(typeid(mock_operation));

	CHECK( id1 == id2 );
}

TEST_CASE("Equal operation_id-s should compre as equal", "[operation_id]")
{
	const auto id1 = operation_id::of<mock_operation>();
	const auto id2 = operation_id::of<mock_operation>();

	CHECK( id1 == id2 );
	CHECK( !(id1 != id2) );
}

TEST_CASE("Unequal operation_id-s should compre as not equal", "[operation_id]")
{
	struct mock_operation1
		: mock_operation
	{
		~mock_operation1() override = default;
	};
	struct mock_operation2
		: mock_operation
	{
		~mock_operation2() override = default;
	};
	
	const auto id1 = operation_id::of<mock_operation1>();
	const auto id2 = operation_id::of<mock_operation2>();

	CHECK( id1 != id2 );
	CHECK( !(id1 == id2) );
}
