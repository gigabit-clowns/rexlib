// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/preprocessing.hpp>

#include <rexlib/em/ops/preprocessing/standardize_background_operation.hpp>

#include "../../functional/fixtures/verb_dispatch_fixture.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::em;
using rexlib::test::verb_dispatch_fixture;
using trompeloeil::_;

namespace
{

/**
 * @brief Whether @p op is a standardize_background_operation with the
 * given radius and bounds.
 */
bool has_parameters(
	const operation &op,
	double radius,
	double lower_bound,
	double upper_bound
)
{
	const auto *typed =
		dynamic_cast<const standardize_background_operation*>(&op);
	return
		typed &&
		typed->get_radius() == radius &&
		typed->get_lower_bound() == lower_bound &&
		typed->get_upper_bound() == upper_bound;
}

} // anonymous namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"standardize_background dispatches a standardize_background_operation "
	"carrying the resolved axes, the radius and the bounds",
	"[em_preprocessing]"
)
{
	std::shared_ptr<mock_buffer> value_storage;
	std::shared_ptr<mock_buffer> seed_storage;
	const array value = make_operand(value_storage, numerical_type::float32);
	const array seed = make_operand(seed_storage, numerical_type::uint64);

	// Referred from the end, so that resolving it is observable.
	const std::vector<std::ptrdiff_t> axes = { -1 };

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<standardize_background_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 2 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == value_storage.get() )
		.LR_WITH( storage_of(_3, 1) == seed_storage.get() )
		.WITH(
			axes_of<standardize_background_operation>(_1) ==
				std::vector<std::size_t>{ 1 }
		)
		.WITH( has_parameters(_1, 5, -3, 4) );

	standardize_background(
		value,
		seed,
		make_span(axes),
		5,
		-3,
		4,
		context,
		nullptr
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"standardize_background_2d standardizes along the last two axes",
	"[em_preprocessing]"
)
{
	std::shared_ptr<mock_buffer> value_storage;
	std::shared_ptr<mock_buffer> seed_storage;
	const array value = make_operand(value_storage, numerical_type::float32);
	const array seed = make_operand(seed_storage, numerical_type::uint64);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<standardize_background_operation>(_1) )
		.WITH( _3.size() == 2 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<standardize_background_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		)
		.WITH( has_parameters(_1, 5, -3, 4) );

	standardize_background_2d(value, seed, 5, -3, 4, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"standardize_background_3d rejects an operand with too few axes",
	"[em_preprocessing]"
)
{
	std::shared_ptr<mock_buffer> value_storage;
	std::shared_ptr<mock_buffer> seed_storage;
	const array value = make_operand(value_storage, numerical_type::float32);
	const array seed = make_operand(seed_storage, numerical_type::uint64);

	// The operand is two dimensional, so there is no third axis.
	CHECK_THROWS_AS(
		standardize_background_3d(value, seed, 5, -3, 4, context, nullptr),
		std::invalid_argument
	);
}
