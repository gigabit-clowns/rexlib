// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <backends/cpu/builders/em/reconstruction/slice_insertion_program_builder.hpp>

#include <backends/cpu/builders/em/reconstruction/unit_weight.hpp>
#include <backends/cpu/hardware/cpu_command_queue.hpp>
#include <backends/cpu/kernels/bessel_i0.hpp>
#include <backends/cpu/kernels/pi.hpp>
#include <core/hardware/host_memory/host_buffer.hpp>

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/ops/reconstruction/insert_slices_operation.hpp>

#include "../../../serial_pool.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <memory>
#include <random>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

using builder_type =
	slice_insertion_program_builder<em::insert_slices_operation, unit_weight>;

template <typename T>
using complex_vector = std::vector<std::complex<T>>;

std::size_t count_elements(const std::vector<std::size_t> &extents)
{
	std::size_t result = 1;
	for (const auto extent : extents)
	{
		result *= extent;
	}
	return result;
}

/**
 * @brief The operands of one insertion, with contiguous layouts.
 */
template <typename T>
class insertion_case
{
public:
	insertion_case(
		std::vector<std::size_t> accumulator_extents,
		std::vector<std::size_t> slice_extents,
		std::vector<std::size_t> transform_extents,
		std::vector<std::size_t> radial_extents,
		unsigned seed
	)
		: m_accumulator_extents(std::move(accumulator_extents))
		, m_slice_extents(std::move(slice_extents))
		, m_transform_extents(std::move(transform_extents))
		, m_radial_extents(std::move(radial_extents))
		, m_numerator(count_elements(m_accumulator_extents))
		, m_denominator(count_elements(m_accumulator_extents))
		, m_slices(count_elements(m_slice_extents))
		, m_transforms(count_elements(m_transform_extents))
		, m_radial(count_elements(m_radial_extents))
	{
		std::mt19937 engine(seed);
		std::uniform_real_distribution<double> unit(-1.0, 1.0);
		std::uniform_real_distribution<double> weight(0.0, 2.0);

		for (auto &value : m_numerator)
		{
			value = std::complex<T>(T(unit(engine)), T(unit(engine)));
		}
		for (auto &value : m_denominator)
		{
			value = T(weight(engine));
		}
		for (auto &value : m_slices)
		{
			value = std::complex<T>(T(unit(engine)), T(unit(engine)));
		}
		for (auto &value : m_transforms)
		{
			value = T(unit(engine));
		}
		for (auto &value : m_radial)
		{
			value = T(weight(engine));
		}
	}

	const std::vector<std::size_t>& get_accumulator_extents() const
	{
		return m_accumulator_extents;
	}

	std::size_t get_slice_extent() const
	{
		return m_slice_extents[m_slice_extents.size() - 2];
	}

	std::size_t get_volume_extent() const
	{
		return m_accumulator_extents[m_accumulator_extents.size() - 3];
	}

	complex_vector<T>& get_numerator() { return m_numerator; }
	std::vector<T>& get_denominator() { return m_denominator; }
	complex_vector<T>& get_slices() { return m_slices; }
	std::vector<T>& get_transforms() { return m_transforms; }
	std::vector<T>& get_radial() { return m_radial; }
	const complex_vector<T>& get_numerator() const { return m_numerator; }
	const std::vector<T>& get_denominator() const { return m_denominator; }
	const complex_vector<T>& get_slices() const { return m_slices; }
	const std::vector<T>& get_transforms() const { return m_transforms; }
	const std::vector<T>& get_radial() const { return m_radial; }

	std::vector<operand_descriptor> make_input_descriptors() const
	{
		return {
			make_descriptor(m_accumulator_extents, complex_type()),
			make_descriptor(m_accumulator_extents, real_type()),
			make_descriptor(m_slice_extents, complex_type()),
			make_descriptor(m_transform_extents, real_type()),
			make_descriptor(m_radial_extents, real_type())
		};
	}

	std::vector<operand_descriptor> make_output_descriptors() const
	{
		return {
			make_descriptor(m_accumulator_extents, complex_type()),
			make_descriptor(m_accumulator_extents, real_type())
		};
	}

private:
	static numerical_type complex_type()
	{
		return numerical_type_of<std::complex<T>>::value;
	}

	static numerical_type real_type()
	{
		return numerical_type_of<T>::value;
	}

	static operand_descriptor make_descriptor(
		const std::vector<std::size_t> &extents,
		numerical_type data_type
	)
	{
		return operand_descriptor(
			strided_layout::make_contiguous_layout(make_span(extents)),
			data_type,
			nullptr
		);
	}

	std::vector<std::size_t> m_accumulator_extents;
	std::vector<std::size_t> m_slice_extents;
	std::vector<std::size_t> m_transform_extents;
	std::vector<std::size_t> m_radial_extents;
	complex_vector<T> m_numerator;
	std::vector<T> m_denominator;
	complex_vector<T> m_slices;
	std::vector<T> m_transforms;
	std::vector<T> m_radial;
};

template <typename T>
std::shared_ptr<host_buffer> make_buffer(const std::vector<T> &values)
{
	auto result = std::make_shared<host_buffer>(
		(values.empty() ? 1 : values.size()) * sizeof(T),
		alignof(T)
	);
	std::copy(
		values.begin(),
		values.end(),
		static_cast<T*>(result->get_host_ptr())
	);
	return result;
}

template <typename T>
std::vector<T> read_buffer(const buffer &buffer, std::size_t count)
{
	const auto *data = static_cast<const T*>(buffer.get_host_ptr());
	return std::vector<T>(data, data + count);
}

/**
 * @brief The result of running the program: the numerator and the
 * denominator.
 */
template <typename T>
class insertion_result
{
public:
	insertion_result(complex_vector<T> numerator, std::vector<T> denominator)
		: m_numerator(std::move(numerator))
		, m_denominator(std::move(denominator))
	{
	}

	const complex_vector<T>& get_numerator() const { return m_numerator; }
	const std::vector<T>& get_denominator() const { return m_denominator; }

private:
	complex_vector<T> m_numerator;
	std::vector<T> m_denominator;
};

/**
 * @brief Build the program of a case and run it.
 *
 * @param in_place Whether the outputs are the accumulators themselves.
 */
template <typename T>
insertion_result<T> run(
	const insertion_case<T> &data,
	const em::gridding_window &window,
	thread_pool &pool,
	bool in_place = false
)
{
	const builder_type builder;
	const em::insert_slices_operation operation(window);
	cpu_command_queue queue(get_serial_pool());
	const auto output_descriptors = data.make_output_descriptors();
	const auto input_descriptors = data.make_input_descriptors();

	const auto program = builder.build(
		operation,
		make_span(output_descriptors),
		make_span(input_descriptors),
		queue,
		nullptr
	);
	REQUIRE( program != nullptr );

	const auto numerator = make_buffer(data.get_numerator());
	const auto denominator = make_buffer(data.get_denominator());
	const std::vector<std::shared_ptr<const buffer>> inputs = {
		numerator,
		denominator,
		make_buffer(data.get_slices()),
		make_buffer(data.get_transforms()),
		make_buffer(data.get_radial())
	};
	const auto count = count_elements(data.get_accumulator_extents());
	const std::vector<std::shared_ptr<buffer>> outputs = {
		in_place ? numerator : make_buffer(complex_vector<T>(count)),
		in_place ? denominator : make_buffer(std::vector<T>(count))
	};
	const std::vector<std::shared_ptr<buffer>> scratches;

	dynamic_cast<cpu_program&>(*program).execute(
		make_span(outputs),
		make_span(inputs),
		make_span(scratches),
		pool
	);

	return insertion_result<T>(
		read_buffer<std::complex<T>>(*outputs[0], count),
		read_buffer<T>(*outputs[1], count)
	);
}

/**
 * @brief The window weight of a voxel at a distance from a sample along
 * one axis, from the formulas of the contract.
 */
double window_weight(const em::gridding_window &window, double distance)
{
	const auto w = window.get_half_width();
	switch (window.get_profile())
	{
	case em::gridding_profile::box:
		return 1.0;
	case em::gridding_profile::triangle:
		return 1.0 - std::abs(distance);
	case em::gridding_profile::kaiser_bessel:
	{
		const auto ratio = distance / w;
		if (!(std::abs(ratio) < 1.0))
		{
			return 0.0;
		}
		const auto shape = window.get_shape();
		return bessel_i0(shape * std::sqrt(1.0 - ratio * ratio)) /
			bessel_i0(shape);
	}
	}
	return 0.0;
}

/**
 * @brief The voxels a sample reaches along one axis.
 */
std::vector<std::ptrdiff_t> reached_voxels(
	const em::gridding_window &window,
	double coordinate
)
{
	switch (window.get_profile())
	{
	case em::gridding_profile::box:
		return { static_cast<std::ptrdiff_t>(std::round(coordinate)) };
	case em::gridding_profile::triangle:
	{
		const auto below = static_cast<std::ptrdiff_t>(std::floor(coordinate));
		return { below, below + 1 };
	}
	case em::gridding_profile::kaiser_bessel:
	{
		std::vector<std::ptrdiff_t> result;
		const auto w = window.get_half_width();
		const auto first =
			static_cast<std::ptrdiff_t>(std::floor(coordinate - w)) + 1;
		const auto last =
			static_cast<std::ptrdiff_t>(std::ceil(coordinate + w)) - 1;
		for (auto g = first; g <= last; ++g)
		{
			result.push_back(g);
		}
		return result;
	}
	}
	return {};
}

/**
 * @brief Insert the slices of a case as the contract states, in double.
 *
 * Every frequency of the full spectrum of each slice is placed, the voxels
 * with a negative x are left out, and the accumulators are the inputs plus
 * the insertions.
 */
template <typename T>
insertion_result<T> reference(
	const insertion_case<T> &data,
	const em::gridding_window &window
)
{
	const auto n = data.get_slice_extent();
	const auto h = n / 2 + 1;
	const auto big = data.get_volume_extent();
	const auto half = big / 2 + 1;
	const auto signed_n = static_cast<std::ptrdiff_t>(n);
	const auto signed_big = static_cast<std::ptrdiff_t>(big);
	const auto scale = static_cast<double>(big) / static_cast<double>(n);
	const auto centre = static_cast<double>(n / 2);
	const auto volume_centre = static_cast<double>(big / 2);
	const auto volume_count = big * big * half;

	// The batches: K accumulators, each receiving B slices whose transforms
	// are laid out (K, B) or (B), as the cases below arrange them.
	const auto &accumulator_extents = data.get_accumulator_extents();
	const auto accumulator_count =
		count_elements(accumulator_extents) / volume_count;
	const auto slice_count = data.get_slices().size() / (n * h);
	const auto transform_count = data.get_transforms().size() / 12;
	const auto radial_count = data.get_radial().size() / h;

	complex_vector<double> numerator(count_elements(accumulator_extents));
	std::vector<double> denominator(numerator.size());
	for (std::size_t i = 0; i < numerator.size(); ++i)
	{
		numerator[i] = data.get_numerator()[i];
		denominator[i] = data.get_denominator()[i];
	}

	for (std::size_t k = 0; k < accumulator_count; ++k)
	{
		const auto slices_per_accumulator =
			transform_count / accumulator_count;
		for (std::size_t b = 0; b < slices_per_accumulator; ++b)
		{
			const auto *slice = &data.get_slices()[
				(b % slice_count) * n * h
			];
			const auto *transform = &data.get_transforms()[
				(k * slices_per_accumulator + b) * 12
			];
			const auto *radial = &data.get_radial()[(b % radial_count) * h];
			auto *num = &numerator[k * volume_count];
			auto *den = &denominator[k * volume_count];

			for (std::ptrdiff_t row = 0; row < signed_n; ++row)
			{
				if (n % 2 == 0 && row == signed_n / 2)
				{
					continue;
				}
				const auto k_y = static_cast<double>(
					row <= signed_n / 2 ? row : row - signed_n
				);
				for (std::ptrdiff_t column = 0;
					column < static_cast<std::ptrdiff_t>(h);
					++column)
				{
					if (n % 2 == 0 && column == signed_n / 2)
					{
						continue;
					}
					const auto k_x = static_cast<double>(column);
					const auto shell = static_cast<std::size_t>(
						std::floor(std::sqrt(k_x * k_x + k_y * k_y) + 0.5)
					);
					if (shell >= h)
					{
						continue;
					}
					const auto r = static_cast<double>(radial[shell]);
					if (r == 0.0)
					{
						continue;
					}

					const std::complex<double> x = slice[row * h + column];
					const auto shift = 2.0 * pi * (
						k_x * (centre + transform[3]) +
						k_y * (centre + transform[7])
					) / static_cast<double>(n);
					const auto v = x * std::polar(1.0, shift);

					for (int sign = 1; sign >= -1; sign -= 2)
					{
						if (sign < 0 && column == 0)
						{
							continue;
						}
						const auto value = sign > 0 ? v : std::conj(v);
						double big_k[3];
						for (std::size_t j = 0; j < 3; ++j)
						{
							big_k[j] = sign * scale * (
								k_x * transform[j] + k_y * transform[4 + j]
							);
						}

						for (const auto gz : reached_voxels(window, big_k[2]))
						for (const auto gy : reached_voxels(window, big_k[1]))
						for (const auto gx : reached_voxels(window, big_k[0]))
						{
							const auto weight = r *
								window_weight(window, gx - big_k[0]) *
								window_weight(window, gy - big_k[1]) *
								window_weight(window, gz - big_k[2]);
							const auto contribution =
								value * weight * std::polar(
								1.0,
								-2.0 * pi * (gx + gy + gz) * volume_centre /
									static_cast<double>(big)
							);
							if (
								gx < 0 || gx > signed_big / 2 ||
								std::abs(gy) > signed_big / 2 ||
								std::abs(gz) > signed_big / 2
							)
							{
								continue;
							}
							const auto sy = gy < 0 ? gy + signed_big : gy;
							const auto sz = gz < 0 ? gz + signed_big : gz;
							const auto index =
								(sz * signed_big + sy) *
									static_cast<std::ptrdiff_t>(half) +
								gx;
							num[index] += contribution;
							den[index] += weight;
						}
					}
				}
			}
		}
	}

	complex_vector<T> numerator_out(numerator.size());
	std::vector<T> denominator_out(denominator.size());
	for (std::size_t i = 0; i < numerator.size(); ++i)
	{
		numerator_out[i] = std::complex<T>(
			static_cast<T>(numerator[i].real()),
			static_cast<T>(numerator[i].imag())
		);
		denominator_out[i] = static_cast<T>(denominator[i]);
	}
	return insertion_result<T>(
		std::move(numerator_out),
		std::move(denominator_out)
	);
}

template <typename T>
void check_close(
	const insertion_result<T> &result,
	const insertion_result<T> &expected,
	double tolerance
)
{
	REQUIRE(
		result.get_numerator().size() == expected.get_numerator().size()
	);
	for (std::size_t i = 0; i < result.get_numerator().size(); ++i)
	{
		INFO(
			"element " << i << ": "
			<< result.get_numerator()[i] << " / "
			<< result.get_denominator()[i] << ", expected "
			<< expected.get_numerator()[i] << " / "
			<< expected.get_denominator()[i]
		);
		CHECK(
			std::abs(result.get_numerator()[i] - expected.get_numerator()[i])
				<= tolerance
		);
		CHECK(
			std::abs(
				result.get_denominator()[i] - expected.get_denominator()[i]
			) <= tolerance
		);
	}
}

template <typename T>
bool identical(
	const insertion_result<T> &a,
	const insertion_result<T> &b
)
{
	return
		a.get_numerator() == b.get_numerator() &&
		a.get_denominator() == b.get_denominator();
}

/**
 * @brief Three slices of extent n into one volume of extent big.
 */
template <typename T>
insertion_case<T> make_batch(std::size_t n, std::size_t big, unsigned seed)
{
	return insertion_case<T>(
		{ big, big, big / 2 + 1 },
		{ 3, n, n / 2 + 1 },
		{ 3, 3, 4 },
		{ 3, n / 2 + 1 },
		seed
	);
}

class size_pair
{
public:
	size_pair(std::size_t slice, std::size_t volume)
		: m_slice(slice)
		, m_volume(volume)
	{
	}

	std::size_t get_slice() const { return m_slice; }
	std::size_t get_volume() const { return m_volume; }

private:
	std::size_t m_slice;
	std::size_t m_volume;
};

} // anonymous namespace

TEST_CASE(
	"the insert_slices program should insert as the contract states, for "
	"either parity and every window",
	"[slice_insertion_program_builder]"
)
{
	const auto sizes = GENERATE(
		size_pair(6, 12), size_pair(5, 7), size_pair(6, 9), size_pair(5, 10)
	);
	const auto window = GENERATE(
		em::gridding_window::nearest(),
		em::gridding_window::linear(),
		em::gridding_window::kaiser_bessel(1.5, 6.0)
	);
	const auto tolerance =
		window.get_profile() == em::gridding_profile::kaiser_bessel
			? 5e-4
			: 1e-9;

	INFO(
		"n = " << sizes.get_slice() << ", N = " << sizes.get_volume()
		<< ", window " << window.get_profile()
	);
	const auto data =
		make_batch<double>(sizes.get_slice(), sizes.get_volume(), 1);

	check_close(
		run(data, window, *get_serial_pool()),
		reference(data, window),
		tolerance
	);
}

TEST_CASE(
	"the insert_slices program should give the same bits on any number of "
	"threads and in place",
	"[slice_insertion_program_builder]"
)
{
	const auto data = make_batch<float>(8, 16, 2);
	const auto window = em::gridding_window::linear();
	thread_pool threads(3);

	const auto serial = run(data, window, *get_serial_pool());
	const auto parallel = run(data, window, threads);
	const auto in_place = run(data, window, threads, true);

	CHECK( identical(serial, parallel) );
	CHECK( identical(serial, in_place) );
	check_close(serial, reference(data, window), 1e-4);
}

TEST_CASE(
	"the insert_slices program should keep one accumulator per kept batch "
	"element and sum the rest",
	"[slice_insertion_program_builder]"
)
{
	// Two accumulators, each receiving the same three slices through
	// transforms of its own.
	const insertion_case<double> data(
		{ 2, 1, 8, 8, 5 },
		{ 3, 6, 4 },
		{ 2, 3, 3, 4 },
		{ 3, 4 },
		3
	);
	const auto window = em::gridding_window::linear();

	check_close(
		run(data, window, *get_serial_pool()),
		reference(data, window),
		1e-9
	);
}

TEST_CASE(
	"the insert_slices program should leave a NaN beyond the cutoff out",
	"[slice_insertion_program_builder]"
)
{
	auto data = make_batch<double>(8, 16, 4);
	const auto not_a_number = std::numeric_limits<double>::quiet_NaN();
	// Shells 3 and 4 of every slice are cut off.
	for (std::size_t b = 0; b < 3; ++b)
	{
		data.get_radial()[b * 5 + 3] = 0.0;
		data.get_radial()[b * 5 + 4] = 0.0;
	}
	// Samples at (k_x, k_y) = (4, 0) are on shell 4.
	for (std::size_t b = 0; b < 3; ++b)
	{
		data.get_slices()[b * 8 * 5 + 4] =
			std::complex<double>(not_a_number, not_a_number);
	}
	const auto window = em::gridding_window::linear();

	const auto result = run(data, window, *get_serial_pool());

	for (const auto value : result.get_numerator())
	{
		CHECK( value == value );
	}
	check_close(result, reference(data, window), 1e-9);
}
