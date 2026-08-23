// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/linalg.hpp>

#include "fixtures/reduction_verb_fixture.hpp"

#include <rex/core/dispatch/execute.hpp>
#include <rex/ops/linalg/matvec_operation.hpp>
#include <rex/ops/linalg/vecdot_operation.hpp>
#include <rex/ops/linalg/vecmat_operation.hpp>

#include <complex>
#include <cstddef>
#include <vector>

using namespace rex;
using namespace rex::ops;
using xmipp4::test::element_value;
using xmipp4::test::reduction_verb_fixture;

// vecdot is a broadcast followed by a sum along an axis, so it shares the
// reduction shape policy and the reduction loop with the reduction family.
// It is the only one of them taking two operands, which is what these cases
// are here to pin.

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecdot contracts two vectors along their last axis",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 3 }, { 1, 2, 3 });
	auto right = make_sequence_operand<float32_t>({ 3 }, { 4, 5, 6 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::vecdot(left_ref, right_ref, context, nullptr),
		{},
		{ 32 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecdot contracts a stack of vectors",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 2, 3 }, { 1, 2, 3, 4, 5, 6 });
	auto right = make_sequence_operand<float32_t>({ 2, 3 }, { 1, 0, 0, 0, 1, 0 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::vecdot(left_ref, right_ref, context, nullptr),
		{ 2 },
		{ 1, 5 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecdot broadcasts the operand that is a single vector",
	"[array_linalg][cpu]"
)
{
	// Every other axis is a batch dimension, so a stack contracts with one
	// vector without that vector being written out.
	auto left = make_sequence_operand<float32_t>({ 2, 3 }, { 1, 2, 3, 4, 5, 6 });
	auto right = make_sequence_operand<float32_t>({ 1, 3 }, { 1, 1, 1 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::vecdot(left_ref, right_ref, context, nullptr),
		{ 2 },
		{ 6, 15 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecdot contracts along an axis other than the last",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 2, 3 }, { 1, 2, 3, 4, 5, 6 });
	auto right = make_sequence_operand<float32_t>({ 2, 3 }, { 1, 1, 1, 2, 2, 2 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::vecdot(left_ref, right_ref, 0, context, nullptr),
		{ 3 },
		{ 9, 12, 15 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecdot conjugates its first operand",
	"[array_linalg][cpu]"
)
{
	// A vector contracted with itself is then its squared magnitude, a real
	// quantity, rather than a complex number. This is what the array API and
	// NumPy's vecdot both do, and it is the only thing separating vecdot
	// from a plain sum of products.
	using complex_type = std::complex<float32_t>;

	auto operand = make_sequence_operand<complex_type>(
		{ 2 },
		{ element_value(3.0, 4.0), element_value(1.0, -2.0) }
	);
	const const_array_ref operand_ref = operand;

	const auto result =
		xmipp4::vecdot(operand_ref, operand_ref, context, nullptr);

	// |3 + 4i|^2 + |1 - 2i|^2 = 25 + 5, with no imaginary part left.
	const auto value = read_host<complex_type>(result, 1).front();
	CHECK( value.real() == 30.0F );
	CHECK( value.imag() == 0.0F );
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matmul multiplies a matrix by a matrix",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 2, 3 }, { 1, 2, 3, 4, 5, 6 }
	);
	auto right = make_sequence_operand<float32_t>(
		{ 3, 4 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::matmul(left_ref, right_ref, context, nullptr),
		{ 2, 4 },
		{ 38, 44, 50, 56, 83, 98, 113, 128 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matmul broadcasts a stack of matrices against a single one",
	"[array_linalg][cpu]"
)
{
	std::vector<element_value> left_values;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v : { 1, 2, 3, 4, 5, 6 })
		{
			left_values.push_back(v);
		}
	}
	auto left = make_sequence_operand<float32_t>({ 5, 2, 3 }, left_values);
	auto right = make_sequence_operand<float32_t>(
		{ 3, 4 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	std::vector<element_value> expected;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v : { 38, 44, 50, 56, 83, 98, 113, 128 })
		{
			expected.push_back(v);
		}
	}

	check_values<float32_t>(
		xmipp4::matmul(left_ref, right_ref, context, nullptr),
		{ 5, 2, 4 },
		expected
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matmul takes the fixed size fast path for small square matrices",
	"[array_linalg][cpu]"
)
{
	// A matrix times the identity is left unchanged, which keeps the
	// expected values trivial to state while still exercising the small,
	// contiguous, row-major fast path this size is meant for.
	auto left = make_sequence_operand<float32_t>(
		{ 3, 3 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }
	);
	auto identity = make_sequence_operand<float32_t>(
		{ 3, 3 }, { 1, 0, 0, 0, 1, 0, 0, 0, 1 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref identity_ref = identity;

	check_values<float32_t>(
		xmipp4::matmul(left_ref, identity_ref, context, nullptr),
		{ 3, 3 },
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matmul takes the fixed size fast path for 4x4 matrices",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 4, 4 },
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
	);
	auto identity = make_sequence_operand<float32_t>(
		{ 4, 4 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref identity_ref = identity;

	check_values<float32_t>(
		xmipp4::matmul(left_ref, identity_ref, context, nullptr),
		{ 4, 4 },
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matmul takes the fixed size fast path for a rectangular matrix",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 3, 4 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
	auto identity = make_sequence_operand<float32_t>(
		{ 4, 4 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref identity_ref = identity;

	check_values<float32_t>(
		xmipp4::matmul(left_ref, identity_ref, context, nullptr),
		{ 3, 4 },
		{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matvec multiplies a matrix by a vector",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 2, 3 }, { 1, 2, 3, 4, 5, 6 }
	);
	auto right = make_sequence_operand<float32_t>({ 3 }, { 1, 1, 1 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		matvec_operation(), left_ref, right_ref, context, nullptr
	);

	check_values<float32_t>(result, { 2 }, { 6, 15 });
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matvec broadcasts a stack of matrices against a single vector",
	"[array_linalg][cpu]"
)
{
	std::vector<element_value> left_values;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v : { 1, 2, 3, 4, 5, 6 })
		{
			left_values.push_back(v);
		}
	}
	auto left = make_sequence_operand<float32_t>({ 5, 2, 3 }, left_values);
	auto right = make_sequence_operand<float32_t>({ 3 }, { 1, 1, 1 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		matvec_operation(), left_ref, right_ref, context, nullptr
	);

	std::vector<element_value> expected;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v : { 6, 15 })
		{
			expected.push_back(v);
		}
	}

	check_values<float32_t>(result, { 5, 2 }, expected);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matvec broadcasts a single matrix against a stack of vectors",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 1, 2, 3 }, { 1, 2, 3, 4, 5, 6 }
	);
	auto right = make_sequence_operand<float32_t>(
		{ 5, 3 },
		{
			1, 0, 0,
			0, 1, 0,
			0, 0, 1,
			1, 1, 0,
			1, 1, 1,
		}
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		matvec_operation(), left_ref, right_ref, context, nullptr
	);

	check_values<float32_t>(
		result,
		{ 5, 2 },
		{ 1, 4, 2, 5, 3, 6, 3, 9, 6, 15 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"matvec does not conjugate its matrix operand",
	"[array_linalg][cpu]"
)
{
	// Unlike vecmat, matvec's matrix is a plain linear map with no
	// inner-product convention attached, so it is left alone. right is a
	// one-hot vector, isolating left's first column verbatim if the matrix
	// is left alone, or its conjugate if it is not.
	using complex_type = std::complex<float32_t>;

	auto left = make_sequence_operand<complex_type>(
		{ 2, 2 },
		{
			element_value(3.0, 4.0), element_value(1.0, -2.0),
			element_value(5.0, 1.0), element_value(2.0, -3.0),
		}
	);
	auto right = make_sequence_operand<complex_type>({ 2 }, { 1, 0 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<complex_type>(
		xmipp4::matvec(left_ref, right_ref, context, nullptr),
		{ 2 },
		{ element_value(3.0, 4.0), element_value(5.0, 1.0) }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecmat multiplies a vector by a matrix",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 3 }, { 1, 1, 1 });
	auto right = make_sequence_operand<float32_t>(
		{ 3, 4 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		vecmat_operation(), left_ref, right_ref, context, nullptr
	);

	check_values<float32_t>(result, { 4 }, { 15, 18, 21, 24 });
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecmat broadcasts a single vector against a stack of matrices",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 3 }, { 1, 1, 1 });

	std::vector<element_value> right_values;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v :
		     { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 })
		{
			right_values.push_back(v);
		}
	}
	auto right = make_sequence_operand<float32_t>({ 5, 3, 4 }, right_values);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		vecmat_operation(), left_ref, right_ref, context, nullptr
	);

	std::vector<element_value> expected;
	for (std::size_t i = 0; i < 5; ++i)
	{
		for (const element_value v : { 15, 18, 21, 24 })
		{
			expected.push_back(v);
		}
	}

	check_values<float32_t>(result, { 5, 4 }, expected);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecmat broadcasts a stack of vectors against a single matrix",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 5, 3 },
		{
			1, 0, 0,
			0, 1, 0,
			0, 0, 1,
			1, 1, 0,
			1, 1, 1,
		}
	);
	auto right = make_sequence_operand<float32_t>(
		{ 1, 3, 4 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	const auto result = xmipp4::execute_binary(
		vecmat_operation(), left_ref, right_ref, context, nullptr
	);

	check_values<float32_t>(
		result,
		{ 5, 4 },
		{
			1, 2, 3, 4,
			5, 6, 7, 8,
			9, 10, 11, 12,
			6, 8, 10, 12,
			15, 18, 21, 24,
		}
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"vecmat conjugates its vector operand",
	"[array_linalg][cpu]"
)
{
	// NumPy's vecmat conjugates the vector it treats as a row, the same
	// convention vecdot uses for its own first operand. Multiplying by the
	// identity isolates exactly this: with no conjugation the result would
	// be the vector itself, unchanged.
	using complex_type = std::complex<float32_t>;

	auto left = make_sequence_operand<complex_type>(
		{ 2 },
		{ element_value(3.0, 4.0), element_value(1.0, -2.0) }
	);
	auto right = make_sequence_operand<complex_type>(
		{ 2, 2 },
		{ 1, 0, 0, 1 }
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<complex_type>(
		xmipp4::vecmat(left_ref, right_ref, context, nullptr),
		{ 2 },
		{ element_value(3.0, -4.0), element_value(1.0, 2.0) }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"cross takes the cross product of two vectors",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>({ 3 }, { 1, 0, 0 });
	auto right = make_sequence_operand<float32_t>({ 3 }, { 0, 1, 0 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::cross(left_ref, right_ref, context, nullptr),
		{ 3 },
		{ 0, 0, 1 }
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"cross broadcasts a stack of vectors against a single one",
	"[array_linalg][cpu]"
)
{
	auto left = make_sequence_operand<float32_t>(
		{ 4, 3 },
		{
			1, 0, 0,
			0, 1, 0,
			0, 0, 1,
			1, 1, 1,
		}
	);
	auto right = make_sequence_operand<float32_t>({ 1, 3 }, { 0, 0, 1 });
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::cross(left_ref, right_ref, context, nullptr),
		{ 4, 3 },
		{
			0, -1, 0,
			1, 0, 0,
			0, 0, 0,
			1, -1, 0,
		}
	);
}

TEST_CASE_METHOD(
	reduction_verb_fixture,
	"cross takes the product along an axis other than the last",
	"[array_linalg][cpu]"
)
{
	// Axis 0 (extent three) is the one the product is taken along; axis 1
	// (extent four) is a batch dimension, the reverse of every other case
	// in this file.
	auto left = make_sequence_operand<float32_t>(
		{ 3, 4 },
		{
			1, 0, 0, 1,
			0, 1, 0, 1,
			0, 0, 1, 1,
		}
	);
	auto right = make_sequence_operand<float32_t>(
		{ 3, 4 },
		{
			0, 0, 0, 0,
			0, 0, 0, 0,
			1, 1, 1, 1,
		}
	);
	const const_array_ref left_ref = left;
	const const_array_ref right_ref = right;

	check_values<float32_t>(
		xmipp4::cross(left_ref, right_ref, 0, context, nullptr),
		{ 3, 4 },
		{
			0, 1, 0, 1,
			-1, 0, 0, -1,
			0, 0, 0, 0,
		}
	);
}
