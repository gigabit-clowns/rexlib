// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/logical.hpp>

#include "fixtures/elementwise_verb_fixture.hpp"

#include <rexlib/ops/logical/logical_and_operation.hpp>
#include <rexlib/ops/logical/logical_not_operation.hpp>
#include <rexlib/ops/logical/logical_or_operation.hpp>
#include <rexlib/ops/logical/logical_xor_operation.hpp>

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::element_value;
using rexlib::test::elementwise_verb_fixture;

// These accept every numerical type, reading each element as true when it
// differs from the zero of its own type, and answer with a boolean.

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_and holds where both elements are true",
	"[array_logical][cpu]"
)
{
	check_binary<logical_and_operation>(
		rexlib::logical_and,
		element_value(1),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_and fails where one element is false",
	"[array_logical][cpu]"
)
{
	check_binary<logical_and_operation>(
		rexlib::logical_and,
		element_value(1),
		element_value(0),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_or holds where one element is true",
	"[array_logical][cpu]"
)
{
	check_binary<logical_or_operation>(
		rexlib::logical_or,
		element_value(0),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_or fails where both elements are false",
	"[array_logical][cpu]"
)
{
	check_binary<logical_or_operation>(
		rexlib::logical_or,
		element_value(0),
		element_value(0),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_xor holds where exactly one element is true",
	"[array_logical][cpu]"
)
{
	check_binary<logical_xor_operation>(
		rexlib::logical_xor,
		element_value(0),
		element_value(1),
		[](auto, auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_xor fails where both elements are true",
	"[array_logical][cpu]"
)
{
	// This is what separates it from logical_or.
	check_binary<logical_xor_operation>(
		rexlib::logical_xor,
		element_value(1),
		element_value(1),
		[](auto, auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_not holds where the element is false",
	"[array_logical][cpu]"
)
{
	check_unary<logical_not_operation>(
		rexlib::logical_not,
		element_value(0),
		[](auto) { return true; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"logical_not fails where the element is true",
	"[array_logical][cpu]"
)
{
	check_unary<logical_not_operation>(
		rexlib::logical_not,
		element_value(1),
		[](auto) { return false; }
	);
}

TEST_CASE_METHOD(
	elementwise_verb_fixture,
	"a complex element is true when only its imaginary part is",
	"[array_logical][cpu]"
)
{
	// The zero of a complex type is the origin, so a number off the real
	// axis is true however small its real part.
	check_unary<logical_not_operation, complex_type_domain>(
		rexlib::logical_not,
		element_value(0, 2),
		[](auto) { return false; }
	);
}
