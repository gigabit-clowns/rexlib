// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/compare.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rex/ops/compare/equal_operation.hpp>
#include <rex/ops/compare/greater_equal_operation.hpp>
#include <rex/ops/compare/greater_operation.hpp>
#include <rex/ops/compare/less_equal_operation.hpp>
#include <rex/ops/compare/less_operation.hpp>
#include <rex/ops/compare/not_equal_operation.hpp>

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::element_value;
using rexlib::test::elementwise_verb_fixture;

// The operands are zero and one throughout. Every admitted type holds both
// and orders them the same way, booleans included, which is what lets one
// case cover a domain reaching from bool to complex.
//
// Each result is a boolean whatever the operands were, so these also cover
// the path where an operation's output type is fixed rather than following
// its input.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"equal holds where the elements agree",
	"[array_compare][cpu]"
)
{
	// Both parts agree, so the complex types answer the same as the rest.
	check_binary<equal_operation>(
		rexlib::equal,
		element_value(1, 2),
		element_value(1, 2),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"equal fails where the elements differ",
	"[array_compare][cpu]"
)
{
	check_binary<equal_operation>(
		rexlib::equal,
		element_value(0),
		element_value(1),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"not_equal holds where the elements differ",
	"[array_compare][cpu]"
)
{
	check_binary<not_equal_operation>(
		rexlib::not_equal,
		element_value(0),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"not_equal fails where the elements agree",
	"[array_compare][cpu]"
)
{
	check_binary<not_equal_operation>(
		rexlib::not_equal,
		element_value(1, 2),
		element_value(1, 2),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"less holds where the first element is smaller",
	"[array_compare][cpu]"
)
{
	check_binary<less_operation>(
		rexlib::less,
		element_value(0),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"less fails the other way round",
	"[array_compare][cpu]"
)
{
	// The operands swapped, so a symmetric comparison would answer the
	// same and this would fail.
	check_binary<less_operation>(
		rexlib::less,
		element_value(1),
		element_value(0),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"less_equal holds where the elements are equal",
	"[array_compare][cpu]"
)
{
	check_binary<less_equal_operation>(
		rexlib::less_equal,
		element_value(1),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"less_equal fails where the first element is larger",
	"[array_compare][cpu]"
)
{
	check_binary<less_equal_operation>(
		rexlib::less_equal,
		element_value(1),
		element_value(0),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"greater holds where the first element is larger",
	"[array_compare][cpu]"
)
{
	check_binary<greater_operation>(
		rexlib::greater,
		element_value(1),
		element_value(0),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"greater fails the other way round",
	"[array_compare][cpu]"
)
{
	check_binary<greater_operation>(
		rexlib::greater,
		element_value(0),
		element_value(1),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"greater_equal holds where the elements are equal",
	"[array_compare][cpu]"
)
{
	check_binary<greater_equal_operation>(
		rexlib::greater_equal,
		element_value(1),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"greater_equal fails where the first element is smaller",
	"[array_compare][cpu]"
)
{
	check_binary<greater_equal_operation>(
		rexlib::greater_equal,
		element_value(0),
		element_value(1),
		[](auto, auto) { return false; }
	);
}
