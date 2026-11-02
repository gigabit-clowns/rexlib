// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/reconstruction.hpp>

#include <rexlib/em/ops/reconstruction/insert_ctf_slices_operation.hpp>
#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>
#include <rexlib/em/ops/reconstruction/insert_weighted_slices_operation.hpp>

#include "../../functional/fixtures/verb_dispatch_fixture.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>

using namespace rexlib;
using namespace rexlib::em;
using rexlib::test::verb_dispatch_fixture;
using trompeloeil::_;

namespace
{

/**
 * @brief Whether @p op is an @p Op carrying a window of the given profile
 * and half-width.
 */
template <typename Op>
bool has_window(
	const operation &op,
	gridding_profile profile,
	double half_width
)
{
	const auto *typed = dynamic_cast<const Op*>(&op);
	return
		typed &&
		typed->get_window().get_profile() == profile &&
		typed->get_window().get_half_width() == half_width;
}

/**
 * @brief Whether @p op is an insert_ctf_slices_operation carrying the given
 * optics and pixel size.
 */
bool has_ctf_parameters(
	const operation &op,
	double voltage,
	double spherical_aberration,
	double amplitude_contrast,
	double pixel_size
)
{
	const auto *typed = dynamic_cast<const insert_ctf_slices_operation*>(&op);
	return
		typed &&
		typed->get_optics().get_voltage() == voltage &&
		typed->get_optics().get_spherical_aberration() ==
			spherical_aberration &&
		typed->get_optics().get_amplitude_contrast() == amplitude_contrast &&
		typed->get_pixel_size() == pixel_size;
}

} // anonymous namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"insert_slices dispatches an insert_slices_operation carrying the window "
	"over the accumulators and the slices",
	"[em_reconstruction]"
)
{
	std::array<std::shared_ptr<mock_buffer>, 5> storages;
	const array numerator = make_operand(
		storages[0],
		numerical_type::complex_float32
	);
	const array denominator = make_operand(storages[1]);
	const array slices = make_operand(
		storages[2],
		numerical_type::complex_float32
	);
	const array transforms = make_operand(storages[3]);
	const array radial_weights = make_operand(storages[4]);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<insert_slices_operation>(_1) )
		.WITH(
			has_window<insert_slices_operation>(
				_1,
				gridding_profile::triangle,
				1.0
			)
		)
		.WITH( _2.size() == 2 )
		.WITH( _3.size() == 5 )
		.LR_WITH( in_session(_4) )
		.WITH( storage_of(_2, 0) == nullptr )
		.WITH( storage_of(_2, 1) == nullptr )
		.LR_WITH( storage_of(_3, 0) == storages[0].get() )
		.LR_WITH( storage_of(_3, 1) == storages[1].get() )
		.LR_WITH( storage_of(_3, 2) == storages[2].get() )
		.LR_WITH( storage_of(_3, 3) == storages[3].get() )
		.LR_WITH( storage_of(_3, 4) == storages[4].get() );

	insert_slices(
		numerator,
		denominator,
		slices,
		transforms,
		radial_weights,
		gridding_window::linear(),
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"insert_slices writes into the outputs it is given",
	"[em_reconstruction]"
)
{
	std::array<std::shared_ptr<mock_buffer>, 5> storages;
	array numerator = make_operand(
		storages[0],
		numerical_type::complex_float32
	);
	array denominator = make_operand(storages[1]);
	const array slices = make_operand(
		storages[2],
		numerical_type::complex_float32
	);
	const array transforms = make_operand(storages[3]);
	const array radial_weights = make_operand(storages[4]);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<insert_slices_operation>(_1) )
		.LR_WITH( storage_of(_2, 0) == storages[0].get() )
		.LR_WITH( storage_of(_2, 1) == storages[1].get() )
		.LR_WITH( storage_of(_3, 0) == storages[0].get() )
		.LR_WITH( storage_of(_3, 1) == storages[1].get() );

	const auto result = insert_slices(
		numerator,
		denominator,
		slices,
		transforms,
		radial_weights,
		gridding_window::nearest(),
		context,
		&numerator,
		&denominator
	);

	CHECK( result.first.get_storage() == storages[0].get() );
	CHECK( result.second.get_storage() == storages[1].get() );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"insert_weighted_slices dispatches an insert_weighted_slices_operation "
	"with the weights after the slices",
	"[em_reconstruction]"
)
{
	std::array<std::shared_ptr<mock_buffer>, 6> storages;
	const array numerator = make_operand(
		storages[0],
		numerical_type::complex_float32
	);
	const array denominator = make_operand(storages[1]);
	const array slices = make_operand(
		storages[2],
		numerical_type::complex_float32
	);
	const array weights = make_operand(storages[3]);
	const array transforms = make_operand(storages[4]);
	const array radial_weights = make_operand(storages[5]);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<insert_weighted_slices_operation>(_1) )
		.WITH(
			has_window<insert_weighted_slices_operation>(
				_1,
				gridding_profile::box,
				0.5
			)
		)
		.WITH( _2.size() == 2 )
		.WITH( _3.size() == 6 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == storages[0].get() )
		.LR_WITH( storage_of(_3, 1) == storages[1].get() )
		.LR_WITH( storage_of(_3, 2) == storages[2].get() )
		.LR_WITH( storage_of(_3, 3) == storages[3].get() )
		.LR_WITH( storage_of(_3, 4) == storages[4].get() )
		.LR_WITH( storage_of(_3, 5) == storages[5].get() );

	insert_weighted_slices(
		numerator,
		denominator,
		slices,
		weights,
		transforms,
		radial_weights,
		gridding_window::nearest(),
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"insert_ctf_slices dispatches an insert_ctf_slices_operation carrying "
	"the optics and the pixel size, with the CTF after the transforms",
	"[em_reconstruction]"
)
{
	std::array<std::shared_ptr<mock_buffer>, 6> storages;
	const array numerator = make_operand(
		storages[0],
		numerical_type::complex_float32
	);
	const array denominator = make_operand(storages[1]);
	const array slices = make_operand(
		storages[2],
		numerical_type::complex_float32
	);
	const array transforms = make_operand(storages[3]);
	const array ctf = make_operand(storages[4]);
	const array radial_weights = make_operand(storages[5]);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<insert_ctf_slices_operation>(_1) )
		.WITH(
			has_window<insert_ctf_slices_operation>(
				_1,
				gridding_profile::kaiser_bessel,
				2.0
			)
		)
		.WITH( has_ctf_parameters(_1, 300, 2.7, 0.1, 1.5) )
		.WITH( _2.size() == 2 )
		.WITH( _3.size() == 6 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == storages[0].get() )
		.LR_WITH( storage_of(_3, 1) == storages[1].get() )
		.LR_WITH( storage_of(_3, 2) == storages[2].get() )
		.LR_WITH( storage_of(_3, 3) == storages[3].get() )
		.LR_WITH( storage_of(_3, 4) == storages[4].get() )
		.LR_WITH( storage_of(_3, 5) == storages[5].get() );

	insert_ctf_slices(
		numerator,
		denominator,
		slices,
		transforms,
		ctf,
		radial_weights,
		gridding_window::kaiser_bessel(2.0, 10.0),
		ctf_optics(300, 2.7, 0.1),
		1.5,
		context
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"insert_ctf_slices rejects a pixel size that is not positive",
	"[em_reconstruction]"
)
{
	std::array<std::shared_ptr<mock_buffer>, 6> storages;
	const array numerator = make_operand(
		storages[0],
		numerical_type::complex_float32
	);
	const array denominator = make_operand(storages[1]);
	const array slices = make_operand(
		storages[2],
		numerical_type::complex_float32
	);
	const array transforms = make_operand(storages[3]);
	const array ctf = make_operand(storages[4]);
	const array radial_weights = make_operand(storages[5]);

	CHECK_THROWS_AS(
		insert_ctf_slices(
			numerator,
			denominator,
			slices,
			transforms,
			ctf,
			radial_weights,
			gridding_window::linear(),
			ctf_optics(300, 2.7, 0.1),
			0.0,
			context
		),
		std::invalid_argument
	);
}
