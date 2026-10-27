// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/em/functional/preprocessing.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/layout/dynamic_subscript.hpp>
#include <rexlib/core/layout/slice.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/functional/view.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

using namespace rexlib;

namespace
{

using extent_list_type = std::vector<std::size_t>;

REXLIB_CONST_CONSTEXPR double infinity =
	std::numeric_limits<double>::infinity();
REXLIB_CONST_CONSTEXPR double not_a_number =
	std::numeric_limits<double>::quiet_NaN();

std::size_t count_of(const extent_list_type &extents)
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
}

/**
 * @brief Smooth values around 100, from -5 to 5 off it.
 */
std::vector<double> make_values(std::size_t count)
{
	std::vector<double> result(count);
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto x = static_cast<double>(i);
		result[i] = 100.0 + 5.0 * std::sin(0.37 * x) * std::cos(0.11 * x);
	}
	return result;
}

template <typename T>
array make_array(
	const execution_context &context,
	const extent_list_type &extents,
	const std::vector<T> &values
)
{
	auto result = empty(
		array_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			numerical_type_of<T>::value
		),
		memory_resource_affinity::device,
		context
	);

	auto *data = static_cast<T*>(result.get_storage()->get_host_ptr());
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		data[i] = values[i];
	}
	return result;
}

template <typename T>
std::vector<double> read_array(const array &result, std::size_t count)
{
	const auto *data =
		static_cast<const T*>(result.get_storage()->get_host_ptr());
	return std::vector<double>(data, data + count);
}

/**
 * @brief Whether the element of an image at a linear index lies in its
 * background.
 */
bool is_background(
	const extent_list_type &core,
	std::size_t element,
	double radius
)
{
	double squared_distance = 0;
	for (std::size_t axis = core.size(); axis > 0; --axis)
	{
		const auto extent = core[axis - 1];
		const auto coordinate = static_cast<double>(element % extent);
		const auto centre = static_cast<double>(extent / 2);
		squared_distance += (coordinate - centre) * (coordinate - centre);
		element /= extent;
	}
	return squared_distance > radius * radius;
}

/**
 * @brief The mean and the scale of the background elements of an image
 * that are finite and whose value, standardized with a previous mean and
 * scale, lies within bounds.
 */
std::array<double, 2> background_moments(
	const double *image,
	const extent_list_type &core,
	double radius,
	const std::array<double, 2> &previous,
	double lower_bound,
	double upper_bound
)
{
	double count = 0;
	double sum = 0;
	double sum_of_squares = 0;
	for (std::size_t i = 0; i < count_of(core); ++i)
	{
		const auto standardized = (image[i] - previous[0]) * previous[1];
		if (
			std::isfinite(image[i]) &&
			is_background(core, i, radius) &&
			standardized >= lower_bound &&
			standardized <= upper_bound
		)
		{
			++count;
			sum += image[i];
			sum_of_squares += image[i] * image[i];
		}
	}

	if (count == 0)
	{
		return {{ 0.0, 1.0 }};
	}

	const auto mean = sum / count;
	const auto variance = sum_of_squares / count - mean * mean;
	return {{ mean, variance > 0 ? 1.0 / std::sqrt(variance) : 1.0 }};
}

/**
 * @brief Standardize images held in the last axes, written plainly.
 *
 * @return std::vector<double> The standardized values, NaN where a draw is
 * expected instead.
 */
std::vector<double> standardize_reference(
	const std::vector<double> &values,
	const extent_list_type &extents,
	std::size_t rank,
	double radius,
	double lower_bound,
	double upper_bound
)
{
	const extent_list_type core(extents.end() - rank, extents.end());
	const auto image_size = count_of(core);

	std::vector<double> result(values.size());
	for (std::size_t first = 0; first < values.size(); first += image_size)
	{
		const auto *image = values.data() + first;
		const auto initial = background_moments(
			image,
			core,
			radius,
			{{ 0.0, 1.0 }},
			-infinity,
			infinity
		);
		const auto last = background_moments(
			image,
			core,
			radius,
			initial,
			lower_bound,
			upper_bound
		);

		for (std::size_t i = 0; i < image_size; ++i)
		{
			const auto standardized = (image[i] - last[0]) * last[1];
			const auto kept =
				std::isfinite(standardized) &&
				standardized >= lower_bound &&
				standardized <= upper_bound;
			result[first + i] = kept ? standardized : not_a_number;
		}
	}
	return result;
}

/**
 * @brief Check a result against a reference holding NaN where a draw is
 * expected.
 */
void check_against(
	const std::vector<double> &result,
	const std::vector<double> &expected,
	double tolerance
)
{
	REQUIRE( result.size() == expected.size() );
	for (std::size_t i = 0; i < result.size(); ++i)
	{
		INFO( "element " << i );
		if (std::isnan(expected[i]))
		{
			CHECK( std::isfinite(result[i]) );
		}
		else
		{
			CHECK(
				std::abs(result[i] - expected[i]) <=
				tolerance * (1.0 + std::abs(expected[i]))
			);
		}
	}
}

/**
 * @brief Three images of 16 x 16, with a stray NaN, a NaN border, an
 * infinity under the particle and a hot pixel in the background.
 */
std::vector<double> make_images()
{
	auto values = make_values(3 * 16 * 16);
	values[0] = not_a_number;
	for (std::size_t x = 0; x < 16; ++x)
	{
		values[256 + 15 * 16 + x] = not_a_number;
	}
	values[8 * 16 + 8] = infinity;
	values[512 + 16 + 1] = 1000;
	return values;
}

template <typename T>
void check_images_against_reference(
	const execution_context &context,
	double tolerance
)
{
	const extent_list_type extents = { 3, 16, 16 };
	const auto images = make_images();
	const std::vector<T> values(images.begin(), images.end());

	const auto result = em::standardize_background_2d(
		make_array(context, extents, values),
		make_array(context, { 3 }, std::vector<std::uint64_t>{ 11, 12, 13 }),
		5,
		-3,
		3,
		context
	);

	check_against(
		read_array<T>(result, values.size()),
		standardize_reference(
			std::vector<double>(values.begin(), values.end()),
			extents,
			2,
			5,
			-3,
			3
		),
		tolerance
	);
}

std::vector<double> standardize_images(
	const execution_context &context,
	const array &value,
	const array &seed,
	std::size_t count
)
{
	return read_array<double>(
		em::standardize_background_2d(value, seed, 5, -3, 3, context),
		count
	);
}

} // anonymous namespace

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d computes what a plain reference computes in "
	"float64",
	"[em_preprocessing][cpu]"
)
{
	check_images_against_reference<double>(context, 1e-12);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d computes what a plain reference computes in "
	"float32",
	"[em_preprocessing][cpu]"
)
{
	check_images_against_reference<float>(context, 1e-5);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_3d computes what a plain reference computes",
	"[em_preprocessing][cpu]"
)
{
	const extent_list_type extents = { 2, 6, 7, 6 };
	auto values = make_values(count_of(extents));
	values[5] = 300;

	const auto result = em::standardize_background_3d(
		make_array(context, extents, values),
		make_array(context, { 2 }, std::vector<std::uint64_t>{ 1, 2 }),
		2,
		-2.5,
		2.5,
		context
	);

	check_against(
		read_array<double>(result, values.size()),
		standardize_reference(values, extents, 3, 2, -2.5, 2.5),
		1e-12
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background standardizes along axes that are not the last",
	"[em_preprocessing][cpu]"
)
{
	const auto values = make_images();
	const auto seed =
		make_array(context, { 3 }, std::vector<std::uint64_t>{ 11, 12, 13 });

	// The same images, with the batch axis between their rows and columns.
	std::vector<double> permuted(values.size());
	for (std::size_t b = 0; b < 3; ++b)
	{
		for (std::size_t y = 0; y < 16; ++y)
		{
			for (std::size_t x = 0; x < 16; ++x)
			{
				permuted[(y * 3 + b) * 16 + x] = values[(b * 16 + y) * 16 + x];
			}
		}
	}

	const auto trailing = standardize_images(
		context,
		make_array(context, { 3, 16, 16 }, values),
		seed,
		values.size()
	);
	const std::vector<std::ptrdiff_t> axes = { 0, 2 };
	const auto around = read_array<double>(
		em::standardize_background(
			make_array(context, { 16, 3, 16 }, permuted),
			seed,
			make_span(axes),
			5,
			-3,
			3,
			context
		),
		values.size()
	);

	for (std::size_t b = 0; b < 3; ++b)
	{
		for (std::size_t y = 0; y < 16; ++y)
		{
			for (std::size_t x = 0; x < 16; ++x)
			{
				INFO( "image " << b << ", row " << y << ", column " << x );
				const auto expected = trailing[(b * 16 + y) * 16 + x];
				const auto actual = around[(y * 3 + b) * 16 + x];
				CHECK( std::abs(actual - expected) <= 1e-12 );
			}
		}
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d reads strided and reversed views",
	"[em_preprocessing][cpu]"
)
{
	auto base = make_array(context, { 3, 32, 32 }, make_values(3 * 32 * 32));
	const std::array<dynamic_subscript, 3> subscripts = {{
		slice(2, 3, -1),
		slice(0, 16, 2),
		slice(1, 16, 2)
	}};
	const auto view = subarray(base, make_span(subscripts));
	const auto contiguous = copy(view, context);
	const auto seed =
		make_array(context, { 3 }, std::vector<std::uint64_t>{ 7, 8, 9 });

	const auto from_view = standardize_images(context, view, seed, 768);
	const auto from_copy = standardize_images(context, contiguous, seed, 768);

	for (std::size_t i = 0; i < from_view.size(); ++i)
	{
		INFO( "element " << i );
		CHECK( std::abs(from_view[i] - from_copy[i]) <= 1e-12 );
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d draws the same values for an image however "
	"it is batched, given a seed per image",
	"[em_preprocessing][cpu]"
)
{
	const auto values = make_images();
	const std::vector<std::uint64_t> seeds = { 11, 12, 13 };

	const auto batched = standardize_images(
		context,
		make_array(context, { 3, 16, 16 }, values),
		make_array(context, { 3 }, seeds),
		values.size()
	);

	for (std::size_t b = 0; b < 3; ++b)
	{
		INFO( "image " << b );
		const std::vector<double> image(
			values.begin() + static_cast<std::ptrdiff_t>(b * 256),
			values.begin() + static_cast<std::ptrdiff_t>((b + 1) * 256)
		);
		const auto alone = standardize_images(
			context,
			make_array(context, { 16, 16 }, image),
			make_array(context, {}, std::vector<std::uint64_t>{ seeds[b] }),
			256
		);
		CHECK(
			alone ==
			std::vector<double>(
				batched.begin() + static_cast<std::ptrdiff_t>(b * 256),
				batched.begin() + static_cast<std::ptrdiff_t>((b + 1) * 256)
			)
		);
	}
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d draws different values for every image "
	"sharing one seed",
	"[em_preprocessing][cpu]"
)
{
	const std::vector<double> values(2 * 4 * 4, not_a_number);

	const auto result = standardize_images(
		context,
		make_array(context, { 2, 4, 4 }, values),
		make_array(context, {}, std::vector<std::uint64_t>{ 5 }),
		values.size()
	);

	for (const auto value : result)
	{
		CHECK( std::isfinite(value) );
	}
	CHECK(
		std::vector<double>(result.begin(), result.begin() + 16) !=
		std::vector<double>(result.begin() + 16, result.end())
	);
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d keeps a hot pixel from inflating the standard "
	"deviation",
	"[em_preprocessing][cpu]"
)
{
	// A checkerboard of -1 and 1, its deviation 1, and a hot pixel.
	std::vector<double> values(16 * 16);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		values[i] = (i / 16 + i % 16) % 2 == 0 ? 1.0 : -1.0;
	}
	values[0] = 1000;

	const auto result = standardize_images(
		context,
		make_array(context, { 16, 16 }, values),
		make_array(context, {}, std::vector<std::uint64_t>{ 3 }),
		values.size()
	);

	CHECK( std::abs(result[0]) < 10 );
	CHECK( std::abs(result[1] + 1.0) < 0.05 );
	CHECK( std::abs(result[2] - 1.0) < 0.05 );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d leaves an image without background "
	"unstandardized",
	"[em_preprocessing][cpu]"
)
{
	std::vector<double> values(8 * 8);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		values[i] = static_cast<double>(i % 5) - 2.0;
	}

	const auto result = read_array<double>(
		em::standardize_background_2d(
			make_array(context, { 8, 8 }, values),
			make_array(context, {}, std::vector<std::uint64_t>{ 1 }),
			100,
			-3,
			3,
			context
		),
		values.size()
	);

	CHECK( result == values );
}

TEST_CASE_METHOD(
	cpu_execution_context_fixture,
	"standardize_background_2d subtracts the value of a constant image",
	"[em_preprocessing][cpu]"
)
{
	const std::vector<double> values(8 * 8, 7.0);

	const auto result = standardize_images(
		context,
		make_array(context, { 8, 8 }, values),
		make_array(context, {}, std::vector<std::uint64_t>{ 1 }),
		values.size()
	);

	CHECK( result == std::vector<double>(values.size(), 0.0) );
}
