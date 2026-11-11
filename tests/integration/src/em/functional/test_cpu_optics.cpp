// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/functional/optics.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/functional/creation.hpp>

#include <cmath>
#include <cstddef>
#include <vector>

using namespace rexlib;

namespace
{

using extent_list_type = std::vector<std::size_t>;

const double pi = 3.141592653589793;

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
 * @brief The CTF of the frequency (k_x, k_y) of an image of n by n pixels.
 */
double reference_ctf(
	const em::ctf_optics &optics,
	double pixel_size,
	std::size_t n,
	const std::vector<double> &parameters,
	double k_x,
	double k_y
)
{
	const auto wavelength = optics.get_wavelength();
	const auto aberration = optics.get_spherical_aberration() * 1e7;
	const auto s = std::hypot(k_x, k_y) /
		(static_cast<double>(n) * pixel_size);
	const auto direction = std::atan2(k_y, k_x);
	const auto defocus = 0.5 * (parameters[0] + parameters[1]) +
		0.5 * (parameters[0] - parameters[1]) *
			std::cos(2.0 * (direction - parameters[2]));
	return std::sin(
		pi * wavelength * defocus * s * s -
		0.5 * pi * aberration * std::pow(wavelength, 3) * std::pow(s, 4) +
		parameters[3] +
		std::asin(optics.get_amplitude_contrast())
	);
}

class optics_fixture
	: public cpu_execution_context_fixture
{
protected:
	template <typename T>
	array make_parameters(
		const extent_list_type &extents,
		const std::vector<double> &values
	) const
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
			data[i] = static_cast<T>(values[i]);
		}
		return result;
	}

	template <typename T>
	std::vector<double> read(const array &values) const
	{
		std::vector<std::size_t> extents;
		values.get_descriptor().get_layout().get_extents(extents);
		const auto host = read_host<T>(values, count_of(extents));
		return std::vector<double>(host.begin(), host.end());
	}

	const em::ctf_optics optics = em::ctf_optics(300, 2.7, 0.1);
	const double pixel_size = 1.5;
};

/**
 * @brief The signed frequency of a row of a half spectrum of extent n.
 */
double row_frequency(std::size_t row, std::size_t n)
{
	return row < (n + 1) / 2
		? static_cast<double>(row)
		: static_cast<double>(row) - static_cast<double>(n);
}

} // anonymous namespace

TEST_CASE_METHOD(
	optics_fixture,
	"ctf_2d should evaluate the CTF of every image at every frequency of "
	"its half spectrum",
	"[em_optics]"
)
{
	const auto n = GENERATE(as<std::size_t>(), 8, 7);
	const auto h = n / 2 + 1;
	const std::vector<std::vector<double>> images = {
		{ 18000, 12000, 0.6, 0 },
		{ 9000, 15000, -1.2, 0.3 }
	};
	std::vector<double> values;
	for (const auto &image : images)
	{
		values.insert(values.end(), image.begin(), image.end());
	}

	const auto result = read<double>(
		em::ctf_2d(
			make_parameters<double>({ 2, 4 }, values),
			n,
			optics,
			pixel_size,
			context
		)
	);

	REQUIRE( result.size() == 2 * n * h );
	for (std::size_t image = 0; image < 2; ++image)
	{
		for (std::size_t row = 0; row < n; ++row)
		{
			for (std::size_t column = 0; column < h; ++column)
			{
				INFO( "image " << image << ", " << row << ", " << column );
				const auto expected = reference_ctf(
					optics,
					pixel_size,
					n,
					images[image],
					static_cast<double>(column),
					row_frequency(row, n)
				);
				const auto value = result[(image * n + row) * h + column];
				CHECK( std::abs(value - expected) < 1e-9 );
			}
		}
	}
}

TEST_CASE_METHOD(
	optics_fixture,
	"ctf_2d should keep the batch axes of the parameters, and evaluate in "
	"float32",
	"[em_optics]"
)
{
	const std::vector<double> image = { 15000, 15000, 0, 0 };
	std::vector<double> values;
	for (std::size_t i = 0; i < 6; ++i)
	{
		values.insert(values.end(), image.begin(), image.end());
	}

	const auto ctf = em::ctf_2d(
		make_parameters<float>({ 2, 3, 4 }, values),
		16,
		optics,
		pixel_size,
		context
	);

	extent_list_type extents;
	ctf.get_descriptor().get_layout().get_extents(extents);
	CHECK( extents == extent_list_type{ 2, 3, 16, 9 } );
	CHECK(
		ctf.get_descriptor().get_data_type() == numerical_type::float32
	);

	const auto result = read<float>(ctf);
	for (std::size_t i = 0; i < result.size(); ++i)
	{
		const auto position = i % (16 * 9);
		const auto expected = reference_ctf(
			optics,
			pixel_size,
			16,
			image,
			static_cast<double>(position % 9),
			row_frequency(position / 9, 16)
		);
		CHECK( std::abs(result[i] - expected) < 1e-4 );
	}
}

TEST_CASE_METHOD(
	optics_fixture,
	"ctf_2d should be the amplitude contrast at the origin with no phase "
	"shift",
	"[em_optics]"
)
{
	const auto result = read<double>(
		em::ctf_2d(
			make_parameters<double>({ 4 }, { 20000, 10000, 0.4, 0 }),
			32,
			optics,
			pixel_size,
			context
		)
	);

	REQUIRE( result.size() == 32 * 17 );
	CHECK( std::abs(result[0] - optics.get_amplitude_contrast()) < 1e-12 );
}

TEST_CASE_METHOD(
	optics_fixture,
	"ctf_1d should be the first row of ctf_2d without astigmatism",
	"[em_optics]"
)
{
	const auto n = GENERATE(as<std::size_t>(), 16, 15);
	const auto h = n / 2 + 1;

	const auto one = read<double>(
		em::ctf_1d(
			make_parameters<double>({ 3, 2 }, {
				10000, 0,
				20000, 0.5,
				30000, -0.2
			}),
			n,
			optics,
			pixel_size,
			context
		)
	);
	const auto two = read<double>(
		em::ctf_2d(
			make_parameters<double>({ 3, 4 }, {
				10000, 10000, 0.9, 0,
				20000, 20000, -0.4, 0.5,
				30000, 30000, 1.7, -0.2
			}),
			n,
			optics,
			pixel_size,
			context
		)
	);

	REQUIRE( one.size() == 3 * h );
	REQUIRE( two.size() == 3 * n * h );
	for (std::size_t signal = 0; signal < 3; ++signal)
	{
		for (std::size_t k = 0; k < h; ++k)
		{
			INFO( "signal " << signal << ", frequency " << k );
			CHECK(
				std::abs(one[signal * h + k] - two[signal * n * h + k]) < 1e-12
			);
		}
	}
}
