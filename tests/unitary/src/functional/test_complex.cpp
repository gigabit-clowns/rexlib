// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/complex.hpp>

#include <rex/ops/complex/angle_operation.hpp>
#include <rex/ops/complex/conjugate_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

#include <stdexcept>

using namespace rex;
using namespace rex::ops;
using rex::test::verb_dispatch_fixture;

namespace
{

std::vector<std::ptrdiff_t> strides_of(const array &value)
{
	std::vector<std::ptrdiff_t> strides;
	value.get_descriptor().get_layout().get_strides(strides);
	return strides;
}

std::vector<std::size_t> extents_of(const array &value)
{
	std::vector<std::size_t> extents;
	value.get_descriptor().get_layout().get_extents(extents);
	return extents;
}

} // namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"real views the real part of a complex array without dispatching",
	"[array_complex]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	array x(storage, make_descriptor({ 2, 3 },
		numerical_type::complex_float32));

	// No expectation is set on the dispatcher: reaching it would be a
	// failure, because this is a reinterpretation and not an operation.
	FORBID_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	));

	const array result = real(x);

	CHECK( result.get_descriptor().get_data_type() ==
	       numerical_type::float32 );
	CHECK( extents_of(result) == extents_of(x) );

	// One float32 step of the view spans one whole complex_float32
	// element, so every stride doubles.
	const auto source_strides = strides_of(x);
	auto expected = source_strides;
	for (auto &stride : expected)
	{
		stride *= 2;
	}
	CHECK( strides_of(result) == expected );
	CHECK( result.get_descriptor().get_layout().get_offset() == 0 );

	// The view aliases its source, which is what makes it a view rather
	// than a copy.
	CHECK( result.get_storage() == storage.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"imag views the imaginary part one element past the real one",
	"[array_complex]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	array x(storage, make_descriptor({ 2, 3 },
		numerical_type::complex_float64));

	FORBID_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	));

	const array result = imag(x);

	CHECK( result.get_descriptor().get_data_type() ==
	       numerical_type::float64 );
	CHECK( extents_of(result) == extents_of(x) );

	// The only difference from the real part is where the view starts.
	CHECK( result.get_descriptor().get_layout().get_offset() == 1 );
	CHECK( result.get_storage() == storage.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"real returns a plain alias of an array that is already real",
	"[array_complex]"
)
{
	const auto storage = std::make_shared<mock_buffer>();
	array x(storage, make_descriptor({ 2, 3 }, numerical_type::float32));

	FORBID_CALL(*dispatcher, dispatch(
		trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_
	));

	const array result = real(x);

	CHECK( result.get_descriptor().get_data_type() ==
	       numerical_type::float32 );
	CHECK( strides_of(result) == strides_of(x) );
	CHECK( result.get_storage() == storage.get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"imag rejects an array that is not complex",
	"[array_complex]"
)
{
	// Unlike the real part, the imaginary part of a real array is not
	// present in its storage, so no view can produce it.
	const auto storage = std::make_shared<mock_buffer>();
	array x(storage, make_descriptor({ 2, 3 }, numerical_type::float32));

	CHECK_THROWS_AS( imag(x), std::invalid_argument );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"conjugate dispatches a conjugate_operation with its single input",
	"[array_complex]"
)
{
	check_unary_verb<conjugate_operation>(conjugate);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"angle dispatches an angle_operation with its single input",
	"[array_complex]"
)
{
	check_unary_verb<angle_operation>(angle);
}
