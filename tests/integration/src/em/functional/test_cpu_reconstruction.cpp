// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <rexlib/em/functional/reconstruction.hpp>

#include "../../functional/fixtures/cpu_execution_context_fixture.hpp"

#include <rexlib/core/hardware/memory_resource_affinity.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/functional/creation.hpp>
#include <rexlib/functional/fourier.hpp>

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <random>
#include <vector>

using namespace rexlib;

namespace
{

using extent_list_type = std::vector<std::size_t>;
using complex_type = std::complex<double>;

std::size_t count_of(const extent_list_type &extents)
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
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
std::vector<T> read_array(const array &values)
{
	std::vector<std::size_t> extents;
	values.get_descriptor().get_layout().get_extents(extents);
	const auto *data =
		static_cast<const T*>(values.get_storage()->get_host_ptr());
	return std::vector<T>(data, data + count_of(extents));
}

/**
 * @brief A cube of random values, (z, y, x) row-major.
 */
std::vector<double> make_volume(std::size_t n, unsigned seed)
{
	std::mt19937 engine(seed);
	std::uniform_real_distribution<double> unit(-1.0, 1.0);
	std::vector<double> result(n * n * n);
	for (auto &value : result)
	{
		value = unit(engine);
	}
	return result;
}

/**
 * @brief The projection of a cube along z: image(y, x) = sum over z.
 */
std::vector<double> project_along_z(
	const std::vector<double> &volume,
	std::size_t n
)
{
	std::vector<double> result(n * n, 0.0);
	for (std::size_t z = 0; z < n; ++z)
	for (std::size_t i = 0; i < n * n; ++i)
	{
		result[i] += volume[z * n * n + i];
	}
	return result;
}

/**
 * @brief The projection of a cube along x, laid out as an image whose x
 * axis is the z axis of the cube: image(y, z) = sum over x.
 */
std::vector<double> project_along_x(
	const std::vector<double> &volume,
	std::size_t n
)
{
	std::vector<double> result(n * n, 0.0);
	for (std::size_t z = 0; z < n; ++z)
	for (std::size_t y = 0; y < n; ++y)
	for (std::size_t x = 0; x < n; ++x)
	{
		result[y * n + z] += volume[(z * n + y) * n + x];
	}
	return result;
}

/**
 * @brief Shift an image circularly by (shift_x, shift_y) pixels.
 */
std::vector<double> shift_image(
	const std::vector<double> &image,
	std::size_t n,
	std::size_t shift_x,
	std::size_t shift_y
)
{
	std::vector<double> result(n * n);
	for (std::size_t y = 0; y < n; ++y)
	for (std::size_t x = 0; x < n; ++x)
	{
		result[((y + shift_y) % n) * n + (x + shift_x) % n] = image[y * n + x];
	}
	return result;
}

/**
 * @brief A 3x4 transform with a rotation, or any 3x3 matrix, and a shift.
 */
std::vector<double> make_transform(
	const std::array<double, 9> &matrix,
	double shift_x,
	double shift_y
)
{
	return {
		matrix[0], matrix[1], matrix[2], shift_x,
		matrix[3], matrix[4], matrix[5], shift_y,
		matrix[6], matrix[7], matrix[8], 0.0
	};
}

const std::array<double, 9> identity = {{ 1, 0, 0, 0, 1, 0, 0, 0, 1 }};

/**
 * @brief The accumulators after one insertion into zeros.
 */
class accumulators
{
public:
	accumulators(
		std::vector<complex_type> numerator,
		std::vector<double> denominator
	)
		: m_numerator(std::move(numerator))
		, m_denominator(std::move(denominator))
	{
	}

	const std::vector<complex_type>& get_numerator() const
	{
		return m_numerator;
	}

	const std::vector<double>& get_denominator() const
	{
		return m_denominator;
	}

private:
	std::vector<complex_type> m_numerator;
	std::vector<double> m_denominator;
};

class reconstruction_fixture
	: public cpu_execution_context_fixture
{
protected:
	/**
	 * @brief Insert images, through their spectra, into zero accumulators.
	 *
	 * @param images The images, (B, n, n) or (n, n).
	 * @param transforms The transforms, (B, 3, 4).
	 */
	accumulators insert(
		const std::vector<double> &images,
		const extent_list_type &image_extents,
		const std::vector<double> &transforms,
		const extent_list_type &transform_extents,
		std::size_t volume_extent,
		const em::gridding_window &window
	)
	{
		const auto n = image_extents.back();
		const auto h = n / 2 + 1;
		const extent_list_type volume = {
			volume_extent, volume_extent, volume_extent / 2 + 1
		};

		const auto slices =
			rfft2(make_array(context, image_extents, images), context);
		auto numerator = zeros(
			make_descriptor(volume, numerical_type::complex_float64),
			memory_resource_affinity::device,
			context
		);
		auto denominator = zeros(
			make_descriptor(volume, numerical_type::float64),
			memory_resource_affinity::device,
			context
		);

		em::insert_slices(
			numerator,
			denominator,
			slices,
			make_array(context, transform_extents, transforms),
			make_array(context, { h }, std::vector<double>(h, 1.0)),
			window,
			context,
			&numerator,
			&denominator
		);

		return accumulators(
			read_array<complex_type>(numerator),
			read_array<double>(denominator)
		);
	}

	std::vector<complex_type> transform_volume(
		const std::vector<double> &volume,
		std::size_t n
	)
	{
		return read_array<complex_type>(
			rfft3(make_array(context, { n, n, n }, volume), context)
		);
	}
};

/**
 * @brief Check that every voxel received either nothing, or once the value
 * of the spectrum of the volume, and that only voxels of a plane did.
 *
 * @param on_plane Whether the stored voxel (z, y, x) lies on the plane.
 */
template <typename OnPlane>
void check_plane_of_spectrum(
	const accumulators &result,
	const std::vector<complex_type> &spectrum,
	std::size_t n,
	const OnPlane &on_plane
)
{
	const auto h = n / 2 + 1;
	std::size_t received = 0;
	for (std::size_t z = 0; z < n; ++z)
	for (std::size_t y = 0; y < n; ++y)
	for (std::size_t x = 0; x < h; ++x)
	{
		const auto index = (z * n + y) * h + x;
		const auto weight = result.get_denominator()[index];
		INFO( "voxel (" << z << ", " << y << ", " << x << ")" );
		if (weight == 0.0)
		{
			CHECK( result.get_numerator()[index] == complex_type(0.0) );
			continue;
		}
		++received;
		CHECK( on_plane(z, y, x) );
		CHECK( weight == 1.0 );
		CHECK(
			std::abs(result.get_numerator()[index] - spectrum[index]) <= 1e-9
		);
	}
	CHECK( received > n );
}

/**
 * @brief A rotation drawn uniformly, from a random unit quaternion.
 */
std::array<double, 9> random_rotation(std::mt19937 &engine)
{
	std::normal_distribution<double> normal;
	double w = normal(engine);
	double x = normal(engine);
	double y = normal(engine);
	double z = normal(engine);
	const auto norm = std::sqrt(w * w + x * x + y * y + z * z);
	w /= norm;
	x /= norm;
	y /= norm;
	z /= norm;
	return {{
		1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y),
		2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x),
		2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)
	}};
}

} // anonymous namespace

TEST_CASE_METHOD(
	reconstruction_fixture,
	"insert_slices should place the spectrum of a projection along z on the "
	"central plane of the spectrum of the volume",
	"[em_reconstruction]"
)
{
	const std::size_t n = GENERATE(7, 8);
	INFO( "n = " << n );
	const auto volume = make_volume(n, 11);

	const auto result = insert(
		project_along_z(volume, n),
		{ n, n },
		make_transform(identity, 0, 0),
		{ 3, 4 },
		n,
		em::gridding_window::nearest()
	);

	check_plane_of_spectrum(
		result,
		transform_volume(volume, n),
		n,
		[](std::size_t z, std::size_t, std::size_t) { return z == 0; }
	);
}

TEST_CASE_METHOD(
	reconstruction_fixture,
	"insert_slices should map the image axes through the rows of the "
	"transform",
	"[em_reconstruction]"
)
{
	// The image x is the volume z and the image y is the volume y, so the
	// projection runs along the volume x and the plane is x = 0.
	const std::size_t n = GENERATE(7, 8);
	INFO( "n = " << n );
	const auto volume = make_volume(n, 12);
	const std::array<double, 9> swap_x_and_z = {{ 0, 0, 1, 0, 1, 0, 1, 0, 0 }};

	const auto result = insert(
		project_along_x(volume, n),
		{ n, n },
		make_transform(swap_x_and_z, 0, 0),
		{ 3, 4 },
		n,
		em::gridding_window::nearest()
	);

	check_plane_of_spectrum(
		result,
		transform_volume(volume, n),
		n,
		[](std::size_t, std::size_t, std::size_t x) { return x == 0; }
	);
}

TEST_CASE_METHOD(
	reconstruction_fixture,
	"insert_slices should undo the shift of a projection given in the "
	"transform",
	"[em_reconstruction]"
)
{
	const std::size_t n = GENERATE(7, 8);
	INFO( "n = " << n );
	const auto image = project_along_z(make_volume(n, 13), n);

	const auto centred = insert(
		image,
		{ n, n },
		make_transform(identity, 0, 0),
		{ 3, 4 },
		n,
		em::gridding_window::nearest()
	);
	const auto shifted = insert(
		shift_image(image, n, 2, n - 3),
		{ n, n },
		make_transform(identity, 2, -3),
		{ 3, 4 },
		n,
		em::gridding_window::nearest()
	);

	CHECK( shifted.get_denominator() == centred.get_denominator() );
	for (std::size_t i = 0; i < centred.get_numerator().size(); ++i)
	{
		INFO( "element " << i );
		CHECK(
			std::abs(shifted.get_numerator()[i] - centred.get_numerator()[i])
				<= 1e-9
		);
	}
}

TEST_CASE_METHOD(
	reconstruction_fixture,
	"insert_slices should reconstruct a Gaussian blob from its projections",
	"[em_reconstruction]"
)
{
	// A Gaussian of width s projects onto a Gaussian of the same width in
	// any direction, so one image serves every orientation.
	const std::size_t n = 16;
	const std::size_t big = 32;
	const double width = 2.0;
	const auto centre = static_cast<double>(n / 2);
	std::vector<double> image(n * n);
	for (std::size_t y = 0; y < n; ++y)
	for (std::size_t x = 0; x < n; ++x)
	{
		const auto dx = static_cast<double>(x) - centre;
		const auto dy = static_cast<double>(y) - centre;
		image[y * n + x] = std::exp(-(dx * dx + dy * dy) / (2 * width * width));
	}

	std::mt19937 engine(14);
	const std::size_t count = 400;
	std::vector<double> transforms;
	for (std::size_t b = 0; b < count; ++b)
	{
		const auto rows = make_transform(random_rotation(engine), 0, 0);
		transforms.insert(transforms.end(), rows.begin(), rows.end());
	}

	const auto result = insert(
		image,
		{ n, n },
		transforms,
		{ count, 3, 4 },
		big,
		em::gridding_window::linear()
	);

	std::vector<complex_type> quotient(result.get_numerator().size());
	for (std::size_t i = 0; i < quotient.size(); ++i)
	{
		const auto weight = result.get_denominator()[i];
		quotient[i] = weight > 1e-3
			? result.get_numerator()[i] / weight
			: complex_type(0.0);
	}
	const auto reconstruction = read_array<double>(
		irfft3(
			make_array(context, { big, big, big / 2 + 1 }, quotient),
			big,
			context
		)
	);

	// The correlation with the blob over the box of the images, centred.
	const auto volume_centre = static_cast<double>(big / 2);
	double product = 0.0;
	double reconstruction_norm = 0.0;
	double blob_norm = 0.0;
	for (std::size_t z = 0; z < big; ++z)
	for (std::size_t y = 0; y < big; ++y)
	for (std::size_t x = 0; x < big; ++x)
	{
		const auto dz = static_cast<double>(z) - volume_centre;
		const auto dy = static_cast<double>(y) - volume_centre;
		const auto dx = static_cast<double>(x) - volume_centre;
		if (std::abs(dz) > centre || std::abs(dy) > centre ||
			std::abs(dx) > centre)
		{
			continue;
		}
		const auto blob = std::exp(
			-(dx * dx + dy * dy + dz * dz) / (2 * width * width)
		);
		const auto value = reconstruction[(z * big + y) * big + x];
		product += value * blob;
		reconstruction_norm += value * value;
		blob_norm += blob * blob;
	}
	const auto correlation =
		product / std::sqrt(reconstruction_norm * blob_norm);

	INFO( "correlation " << correlation );
	CHECK( correlation > 0.99 );
}

TEST_CASE_METHOD(
	reconstruction_fixture,
	"insert_slices should give the same bits for a batch split across calls "
	"into the same accumulators",
	"[em_reconstruction]"
)
{
	const std::size_t n = 12;
	const std::size_t big = 24;
	const std::size_t count = 40;
	const auto h = n / 2 + 1;
	const extent_list_type volume = { big, big, big / 2 + 1 };

	std::mt19937 engine(15);
	std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
	std::vector<float> images(count * n * n);
	for (auto &value : images)
	{
		value = unit(engine);
	}
	std::vector<float> transforms;
	for (std::size_t b = 0; b < count; ++b)
	{
		for (const auto value : make_transform(random_rotation(engine), 0, 0))
		{
			transforms.push_back(static_cast<float>(value));
		}
	}
	const auto slices =
		rfft2(make_array(context, { count, n, n }, images), context);
	const auto all_transforms =
		make_array(context, { count, 3, 4 }, transforms);
	const auto radial =
		make_array(context, { h }, std::vector<float>(h, 1.0f));
	const auto window = em::gridding_window::linear();

	const auto make_accumulators = [&]()
	{
		return std::make_pair(
			zeros(
				make_descriptor(volume, numerical_type::complex_float32),
				memory_resource_affinity::device,
				context
			),
			zeros(
				make_descriptor(volume, numerical_type::float32),
				memory_resource_affinity::device,
				context
			)
		);
	};

	auto whole = make_accumulators();
	em::insert_slices(
		whole.first, whole.second, slices, all_transforms, radial, window,
		context, &whole.first, &whole.second
	);

	auto split = make_accumulators();
	const auto slice_values = read_array<std::complex<float>>(slices);
	const auto half = count / 2;
	for (std::size_t part = 0; part < 2; ++part)
	{
		const std::vector<std::complex<float>> part_slices(
			slice_values.begin() + part * half * n * h,
			slice_values.begin() + (part + 1) * half * n * h
		);
		const std::vector<float> part_transforms(
			transforms.begin() + part * half * 12,
			transforms.begin() + (part + 1) * half * 12
		);
		em::insert_slices(
			split.first,
			split.second,
			make_array(context, { half, n, h }, part_slices),
			make_array(context, { half, 3, 4 }, part_transforms),
			radial,
			window,
			context,
			&split.first,
			&split.second
		);
	}

	CHECK(
		read_array<std::complex<float>>(split.first) ==
		read_array<std::complex<float>>(whole.first)
	);
	CHECK( read_array<float>(split.second) == read_array<float>(whole.second) );
}
