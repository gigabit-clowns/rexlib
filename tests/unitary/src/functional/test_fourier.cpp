// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/functional/fourier.hpp>

#include <rexlib/ops/fourier/fft_operation.hpp>
#include <rexlib/ops/fourier/fftshift_operation.hpp>
#include <rexlib/ops/fourier/ifft_operation.hpp>
#include <rexlib/ops/fourier/ifftshift_operation.hpp>
#include <rexlib/ops/fourier/irfft_operation.hpp>
#include <rexlib/ops/fourier/rfft_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rexlib;
using namespace rexlib::ops;
using rexlib::test::verb_dispatch_fixture;
using trompeloeil::_;

namespace
{

// The operand is two dimensional, so the last axis is axis 1.
REXLIB_CONST_CONSTEXPR std::size_t last_axis = 1;

/**
 * @brief Get the extent an irfft_operation restores its signal to, or 0 when
 * @p op is not one.
 */
std::size_t restored_extent_of(const operation &op)
{
	const auto *irfft = dynamic_cast<const irfft_operation*>(&op);
	return irfft ? irfft->get_parameters().get_extent() : 0;
}

} // anonymous namespace

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fft dispatches a fft_operation carrying the resolved axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// Referred from the end, so that resolving it is observable.
	const std::vector<std::ptrdiff_t> axes = { -1 };

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == x_storage.get() )
		.WITH(
			axes_of<fft_operation>(_1) ==
				std::vector<std::size_t>{ last_axis }
		);

	fft(x, make_span(axes), context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fft2 transforms along the last two axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<fft_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		);

	fft2(x, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fft3 rejects an operand with too few axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// The operand is two dimensional, so there is no third axis to
	// transform along.
	CHECK_THROWS_AS( fft3(x, context, nullptr), std::invalid_argument );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ifft dispatches a ifft_operation carrying the resolved axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::complex_float32);
	const const_array_ref x = x_array;

	// Referred from the end, so that resolving it is observable.
	const std::vector<std::ptrdiff_t> axes = { -1 };

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<ifft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == x_storage.get() )
		.WITH(
			axes_of<ifft_operation>(_1) ==
				std::vector<std::size_t>{ last_axis }
		);

	ifft(x, make_span(axes), context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ifft2 transforms along the last two axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::complex_float32);
	const const_array_ref x = x_array;

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<ifft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<ifft_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		);

	ifft2(x, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ifft3 rejects an operand with too few axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::complex_float32);
	const const_array_ref x = x_array;

	// The operand is two dimensional, so there is no third axis to
	// transform along.
	CHECK_THROWS_AS( ifft3(x, context, nullptr), std::invalid_argument );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"rfft dispatches a rfft_operation carrying the resolved axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// Referred from the end, so that resolving it is observable.
	const std::vector<std::ptrdiff_t> axes = { -1 };

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<rfft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.LR_WITH( storage_of(_3, 0) == x_storage.get() )
		.WITH(
			axes_of<rfft_operation>(_1) ==
				std::vector<std::size_t>{ last_axis }
		);

	rfft(x, make_span(axes), context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"rfft2 transforms along the last two axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<rfft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<rfft_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		);

	rfft2(x, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"rfft3 rejects an operand with too few axes",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage, numerical_type::float32);
	const const_array_ref x = x_array;

	// The operand is two dimensional, so there is no third axis to
	// transform along.
	CHECK_THROWS_AS( rfft3(x, context, nullptr), std::invalid_argument );
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"irfft hands the extent it is given to the operation",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array =
		make_operand(x_storage, numerical_type::complex_float32);
	const const_array_ref x = x_array;

	const std::vector<std::ptrdiff_t> axes = { -1 };

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<irfft_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH( restored_extent_of(_1) == 17 );

	irfft(x, make_span(axes), 17, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"fftshift without axes shifts along every one of them",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage);
	const const_array_ref x = x_array;

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<fftshift_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<fftshift_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		);

	fftshift(x, context, nullptr);
}

TEST_CASE_METHOD(
	verb_dispatch_fixture,
	"ifftshift without axes shifts along every one of them",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array = make_operand(x_storage);
	const const_array_ref x = x_array;

	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.WITH( is_operation<ifftshift_operation>(_1) )
		.WITH( _2.size() == 1 )
		.WITH( _3.size() == 1 )
		.LR_WITH( in_session(_4) )
		.WITH(
			axes_of<ifftshift_operation>(_1) ==
				std::vector<std::size_t>{ 0, 1 }
		);

	ifftshift(x, context, nullptr);
}
