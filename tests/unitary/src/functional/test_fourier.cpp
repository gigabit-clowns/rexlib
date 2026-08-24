// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/functional/fourier.hpp>

#include <rex/ops/fourier/fft_operation.hpp>
#include <rex/ops/fourier/fftshift_operation.hpp>
#include <rex/ops/fourier/ifft_operation.hpp>
#include <rex/ops/fourier/ifftshift_operation.hpp>
#include <rex/ops/fourier/irfft_operation.hpp>
#include <rex/ops/fourier/rfft_operation.hpp>

#include "fixtures/verb_dispatch_fixture.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

using namespace rex;
using namespace rex::ops;
using rex::test::verb_dispatch_fixture;
using trompeloeil::_;

namespace
{

// The operand is two dimensional, so the last axis is axis 1.
REXLIB_CONST_CONSTEXPR std::size_t last_axis = 1;

} // namespace

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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const fft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	fft(x, make_span(axes), context, nullptr);

	check_record<fft_operation>(record, 1);
	CHECK( record.get_input_storage(0) == x_storage.get() );
	REQUIRE( seen.size() == 1 );
	CHECK( seen[0] == last_axis );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const fft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	fft2(x, context, nullptr);

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	check_record<fft_operation>(record, 1);
	REQUIRE( seen.size() == 2 );
	CHECK( seen[0] == 0 );
	CHECK( seen[1] == 1 );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const ifft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	ifft(x, make_span(axes), context, nullptr);

	check_record<ifft_operation>(record, 1);
	CHECK( record.get_input_storage(0) == x_storage.get() );
	REQUIRE( seen.size() == 1 );
	CHECK( seen[0] == last_axis );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const ifft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	ifft2(x, context, nullptr);

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	check_record<ifft_operation>(record, 1);
	REQUIRE( seen.size() == 2 );
	CHECK( seen[0] == 0 );
	CHECK( seen[1] == 1 );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const rfft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	rfft(x, make_span(axes), context, nullptr);

	check_record<rfft_operation>(record, 1);
	CHECK( record.get_input_storage(0) == x_storage.get() );
	REQUIRE( seen.size() == 1 );
	CHECK( seen[0] == last_axis );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const rfft_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	rfft2(x, context, nullptr);

	// The numbered spellings are sugar over the one primitive: they name
	// the trailing axes and leave the rest as batch dimensions.
	check_record<rfft_operation>(record, 1);
	REQUIRE( seen.size() == 2 );
	CHECK( seen[0] == 0 );
	CHECK( seen[1] == 1 );
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
	"irfft turns the extent it is given into the parity it carries",
	"[array_fourier]"
)
{
	std::shared_ptr<mock_buffer> x_storage;
	const array x_array =
		make_operand(x_storage, numerical_type::complex_float32);
	const const_array_ref x = x_array;

	const std::vector<std::ptrdiff_t> axes = { -1 };

	test::dispatch_record record;
	signal_parity seen = signal_parity::even;
	record.inspect = [&seen] (const operation &op)
	{
		seen = dynamic_cast<const irfft_operation&>(op)
			.get_shape_policy().get_parity();
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	// An operation carries the parity, that being all the restoration
	// needs. Naming an extent is the convenience spelling of it.
	irfft(x, make_span(axes), 17, context, nullptr);

	check_record<irfft_operation>(record, 1);
	CHECK( seen == signal_parity::odd );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const fftshift_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	fftshift(x, context, nullptr);

	check_record<fftshift_operation>(record, 1);
	REQUIRE( seen.size() == 2 );
	CHECK( seen[0] == 0 );
	CHECK( seen[1] == 1 );
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

	test::dispatch_record record;
	std::vector<std::size_t> seen;
	record.inspect = [&seen] (const operation &op)
	{
		const auto &typed = dynamic_cast<const ifftshift_operation&>(op);
		const auto axes = typed.get_shape_policy().get_axes();
		seen.assign(axes.begin(), axes.end());
	};
	REQUIRE_CALL(*dispatcher, dispatch(_, _, _, _))
		.LR_SIDE_EFFECT( record(_1, _2, _3, _4) );

	ifftshift(x, context, nullptr);

	check_record<ifftshift_operation>(record, 1);
	REQUIRE( seen.size() == 2 );
	CHECK( seen[0] == 0 );
	CHECK( seen[1] == 1 );
}
