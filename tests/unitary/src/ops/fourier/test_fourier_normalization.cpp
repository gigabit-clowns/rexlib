// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <rex/ops/fourier/fourier_normalization.hpp>

#include <cmath>
#include <cstddef>
#include <sstream>

using namespace rexlib;
using namespace rexlib::ops;
using Catch::Matchers::WithinRel;

namespace
{

double scale(
	fourier_normalization normalization,
	fourier_direction direction,
	std::size_t count
)
{
	return get_fourier_scale(normalization, direction, count);
}

} // namespace

TEST_CASE(
	"get_fourier_scale leaves the forward transform alone by default",
	"[fourier_normalization]"
)
{
	CHECK(
		scale(
			fourier_normalization::backward,
			fourier_direction::forward,
			8
		) == 1.0
	);
}

TEST_CASE(
	"get_fourier_scale divides the inverse transform by default",
	"[fourier_normalization]"
)
{
	CHECK_THAT(
		scale(
			fourier_normalization::backward,
			fourier_direction::inverse,
			8
		),
		WithinRel(0.125, 1e-15)
	);
}

TEST_CASE(
	"get_fourier_scale moves the scaling onto the forward transform",
	"[fourier_normalization]"
)
{
	// The mirror image of the default: the zero frequency of a forward
	// transform becomes the mean of the signal rather than its total.
	CHECK_THAT(
		scale(fourier_normalization::forward, fourier_direction::forward, 8),
		WithinRel(0.125, 1e-15)
	);
	CHECK(
		scale(
			fourier_normalization::forward,
			fourier_direction::inverse,
			8
		) == 1.0
	);
}

TEST_CASE(
	"get_fourier_scale splits the scaling evenly for an orthonormal transform",
	"[fourier_normalization]"
)
{
	// The one convention that is its own mirror image, which is why the
	// direction does not come into it.
	const auto expected = 1.0 / std::sqrt(8.0);

	CHECK_THAT(
		scale(fourier_normalization::ortho, fourier_direction::forward, 8),
		WithinRel(expected, 1e-15)
	);
	CHECK_THAT(
		scale(fourier_normalization::ortho, fourier_direction::inverse, 8),
		WithinRel(expected, 1e-15)
	);
}

TEST_CASE(
	"get_fourier_scale spreads the same total over every convention",
	"[fourier_normalization]"
)
{
	// The property the whole vocabulary exists for. A transform followed by
	// its inverse multiplies a signal by the number of samples, so the two
	// scale factors have to multiply to its reciprocal. That they do so under
	// every convention is what makes a round trip an identity whichever one
	// the caller names, without the caller having to pair two different ones
	// up.
	const std::size_t count = 12;
	const auto conventions = {
		fourier_normalization::backward,
		fourier_normalization::ortho,
		fourier_normalization::forward
	};

	for (const auto normalization : conventions)
	{
		INFO( "convention " << normalization );
		const auto round_trip =
			scale(normalization, fourier_direction::forward, count) *
			scale(normalization, fourier_direction::inverse, count);

		CHECK_THAT( round_trip, WithinRel(1.0 / count, 1e-15) );
	}
}

TEST_CASE(
	"get_fourier_scale leaves a single sample untouched",
	"[fourier_normalization]"
)
{
	// A transform along no axis at all reaches each value with one sample, so
	// there is no convention under which it scales anything.
	const auto conventions = {
		fourier_normalization::backward,
		fourier_normalization::ortho,
		fourier_normalization::forward
	};

	for (const auto normalization : conventions)
	{
		INFO( "convention " << normalization );
		CHECK( scale(normalization, fourier_direction::forward, 1) == 1.0 );
		CHECK( scale(normalization, fourier_direction::inverse, 1) == 1.0 );
	}
}

TEST_CASE(
	"get_fourier_scale answers for a transform of nothing",
	"[fourier_normalization]"
)
{
	// There is no value to scale, and dividing by the count would be dividing
	// by zero, so the answer is the factor that changes nothing.
	CHECK(
		scale(
			fourier_normalization::ortho,
			fourier_direction::inverse,
			0
		) == 1.0
	);
}

TEST_CASE(
	"a scaling convention names itself",
	"[fourier_normalization]"
)
{
	std::ostringstream oss;
	oss << fourier_normalization::backward
		<< fourier_normalization::ortho
		<< fourier_normalization::forward;

	CHECK( oss.str() == "backwardorthoforward" );
}

TEST_CASE(
	"a transform direction names itself",
	"[fourier_normalization]"
)
{
	std::ostringstream oss;
	oss << fourier_direction::forward << fourier_direction::inverse;

	CHECK( oss.str() == "forwardinverse" );
}
