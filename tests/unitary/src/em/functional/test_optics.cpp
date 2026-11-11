// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/optics.hpp>

#include <rexlib/em/ops/optics/ctf_1d_operation.hpp>
#include <rexlib/em/ops/optics/ctf_2d_operation.hpp>

#include "../../functional/fixtures/verb_dispatch_fixture.hpp"

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
 * @brief Whether @p op is an Op carrying the given extent, optics and pixel
 * size.
 */
template <typename Op>
bool has_ctf_parameters(
	const operation &op,
	std::size_t extent,
	double voltage,
	double spherical_aberration,
	double amplitude_contrast,
	double pixel_size
)
{
	const auto *typed = dynamic_cast<const Op*>(&op);
	return
		typed &&
		typed->get_parameters().get_extent() == extent &&
		typed->get_optics().get_voltage() == voltage &&
		typed->get_optics().get_spherical_aberration() ==
			spherical_aberration &&
		typed->get_optics().get_amplitude_contrast() == amplitude_contrast &&
		typed->get_pixel_size() == pixel_size;
}

} // anonymous namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ctf_1d dispatches a ctf_1d_operation carrying the extent, the optics "
	"and the pixel size",
	"[em_optics]"
)
{
	std::shared_ptr<mock_buffer> storage;
	const array parameters = make_operand(storage);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<ctf_1d_operation>(_1) )
		.WITH(
			has_ctf_parameters<ctf_1d_operation>(_1, 64, 300, 2.7, 0.1, 1.5)
		)
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == storage.get() );

	ctf_1d(parameters, 64, ctf_optics(300, 2.7, 0.1), 1.5, context);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ctf_1d rejects a pixel size that is not positive",
	"[em_optics]"
)
{
	std::shared_ptr<mock_buffer> storage;
	const array parameters = make_operand(storage);

	CHECK_THROWS_AS(
		ctf_1d(parameters, 64, ctf_optics(300, 2.7, 0.1), 0.0, context),
		std::invalid_argument
	);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ctf_2d dispatches a ctf_2d_operation carrying the extent, the optics "
	"and the pixel size",
	"[em_optics]"
)
{
	std::shared_ptr<mock_buffer> storage;
	const array parameters = make_operand(storage);

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<ctf_2d_operation>(_1) )
		.WITH(
			has_ctf_parameters<ctf_2d_operation>(_1, 64, 300, 2.7, 0.1, 1.5)
		)
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == storage.get() );

	ctf_2d(parameters, 64, ctf_optics(300, 2.7, 0.1), 1.5, context);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ctf_2d rejects a pixel size that is not positive",
	"[em_optics]"
)
{
	std::shared_ptr<mock_buffer> storage;
	const array parameters = make_operand(storage);

	CHECK_THROWS_AS(
		ctf_2d(parameters, 64, ctf_optics(300, 2.7, 0.1), 0.0, context),
		std::invalid_argument
	);
}
