// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <catch2/catch_test_macros.hpp>

#include <core/hardware/host_memory/host_buffer.hpp>

#include <rexlib/backends/cpu/cpu_program.hpp>
#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/dispatch/operand_descriptor.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_traits.hpp>
#include <rexlib/core/platform/constexpr.hpp>
#include <rexlib/core/span.hpp>
#include <rexlib/em/ops/parameters/ctf_optics.hpp>
#include <rexlib/em/ops/parameters/gridding_window.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <memory>
#include <ostream>
#include <random>
#include <utility>
#include <vector>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/bessel.hpp>

namespace rexlib
{
namespace test
{

using boost::math::cyl_bessel_i;
using boost::math::double_constants::pi;


template <typename T>
using complex_vector = std::vector<std::complex<T>>;

/**
 * @brief Which of the three insertions a case runs.
 */
enum class insertion_variant
{
	plain,
	weighted,
	ctf
};

inline
std::ostream& operator<<(std::ostream &os, insertion_variant variant)
{
	switch (variant)
	{
	case insertion_variant::plain: return os << "plain";
	case insertion_variant::weighted: return os << "weighted";
	case insertion_variant::ctf: return os << "ctf";
	}
	return os;
}

REXLIB_CONST_CONSTEXPR double case_pixel_size = 1.5;

inline
em::ctf_optics make_optics()
{
	return em::ctf_optics(300, 2.7, 0.1);
}

inline
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
 * @brief The extents of the operands of one insertion.
 */
class case_extents
{
public:
	case_extents(
		std::vector<std::size_t> accumulator,
		std::vector<std::size_t> slices,
		std::vector<std::size_t> transforms,
		std::vector<std::size_t> radial,
		std::vector<std::size_t> extra
	)
		: m_accumulator(std::move(accumulator))
		, m_slices(std::move(slices))
		, m_transforms(std::move(transforms))
		, m_radial(std::move(radial))
		, m_extra(std::move(extra))
	{
	}

	const std::vector<std::size_t>& get_accumulator() const
	{
		return m_accumulator;
	}
	const std::vector<std::size_t>& get_slices() const { return m_slices; }
	const std::vector<std::size_t>& get_transforms() const
	{
		return m_transforms;
	}
	const std::vector<std::size_t>& get_radial() const { return m_radial; }
	const std::vector<std::size_t>& get_extra() const { return m_extra; }

private:
	std::vector<std::size_t> m_accumulator;
	std::vector<std::size_t> m_slices;
	std::vector<std::size_t> m_transforms;
	std::vector<std::size_t> m_radial;
	std::vector<std::size_t> m_extra;
};

/**
 * @brief The operands of one insertion, with contiguous layouts.
 *
 * The extra operand is the weights of insert_weighted_slices or the CTF
 * parameters of insert_ctf_slices, and is empty for insert_slices.
 */
template <typename T>
class insertion_case
{
public:
	insertion_case(
		insertion_variant variant,
		case_extents extents,
		unsigned seed
	)
		: m_variant(variant)
		, m_extents(std::move(extents))
		, m_numerator(count_elements(m_extents.get_accumulator()))
		, m_denominator(count_elements(m_extents.get_accumulator()))
		, m_slices(count_elements(m_extents.get_slices()))
		, m_transforms(count_elements(m_extents.get_transforms()))
		, m_radial(count_elements(m_extents.get_radial()))
		, m_extra(
			variant == insertion_variant::plain
				? 0
				: count_elements(m_extents.get_extra())
		)
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

		if (variant == insertion_variant::weighted)
		{
			for (auto &value : m_extra)
			{
				value = T(weight(engine));
			}
		}
		else if (variant == insertion_variant::ctf)
		{
			std::uniform_real_distribution<double> defocus(8000, 25000);
			std::uniform_real_distribution<double> angle(0, pi);
			std::uniform_real_distribution<double> shift(0, 0.5);
			for (std::size_t i = 0; i < m_extra.size(); i += 4)
			{
				m_extra[i] = T(defocus(engine));
				m_extra[i + 1] = T(defocus(engine));
				m_extra[i + 2] = T(angle(engine));
				m_extra[i + 3] = T(shift(engine));
			}
		}
	}

	insertion_variant get_variant() const { return m_variant; }
	const case_extents& get_extents() const { return m_extents; }

	std::size_t get_slice_extent() const
	{
		const auto &slices = m_extents.get_slices();
		return slices[slices.size() - 2];
	}

	std::size_t get_volume_extent() const
	{
		const auto &accumulator = m_extents.get_accumulator();
		return accumulator[accumulator.size() - 3];
	}

	const complex_vector<T>& get_numerator() const { return m_numerator; }
	const std::vector<T>& get_denominator() const { return m_denominator; }
	const complex_vector<T>& get_slices() const { return m_slices; }
	const std::vector<T>& get_transforms() const { return m_transforms; }
	const std::vector<T>& get_radial() const { return m_radial; }
	const std::vector<T>& get_extra() const { return m_extra; }
	complex_vector<T>& get_slices() { return m_slices; }
	std::vector<T>& get_transforms() { return m_transforms; }
	std::vector<T>& get_radial() { return m_radial; }

private:
	insertion_variant m_variant;
	case_extents m_extents;
	complex_vector<T> m_numerator;
	std::vector<T> m_denominator;
	complex_vector<T> m_slices;
	std::vector<T> m_transforms;
	std::vector<T> m_radial;
	std::vector<T> m_extra;
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

template <typename T>
operand_descriptor make_descriptor(
	const std::vector<std::size_t> &extents,
	bool complex
)
{
	return operand_descriptor(
		strided_layout::make_contiguous_layout(make_span(extents)),
		complex
			? numerical_type_of<std::complex<T>>::value
			: numerical_type_of<T>::value,
		nullptr
	);
}

/**
 * @brief The numerator and the denominator.
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
 * @brief The window weight of a voxel at a distance from a sample along
 * one axis, from the formulas of the contract.
 */
inline
double window_weight(const em::gridding_window &window, double distance)
{
	switch (window.get_profile())
	{
	case em::gridding_profile::box:
		return 1.0;
	case em::gridding_profile::triangle:
		return 1.0 - std::abs(distance);
	case em::gridding_profile::kaiser_bessel:
	{
		const auto ratio = distance / window.get_half_width();
		if (!(std::abs(ratio) < 1.0))
		{
			return 0.0;
		}
		const auto shape = window.get_shape();
		return cyl_bessel_i(0, shape * std::sqrt(1.0 - ratio * ratio)) /
			cyl_bessel_i(0, shape);
	}
	}
	return 0.0;
}

/**
 * @brief The voxels a sample reaches along one axis.
 */
inline
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
 * @brief The CTF of a frequency, from the formula of the contract.
 */
inline
double reference_ctf(
	const double *parameters,
	std::size_t extent,
	double k_x,
	double k_y
)
{
	const auto optics = make_optics();
	const auto wavelength = optics.get_wavelength();
	const auto aberration = optics.get_spherical_aberration() * 1e7;
	const auto s2 = (k_x * k_x + k_y * k_y) /
		std::pow(static_cast<double>(extent) * case_pixel_size, 2);
	const auto direction = std::atan2(k_y, k_x);
	const auto defocus = 0.5 * (parameters[0] + parameters[1]) +
		0.5 * (parameters[0] - parameters[1]) *
			std::cos(2.0 * (direction - parameters[2]));
	return std::sin(
		pi * wavelength * defocus * s2 -
		0.5 * pi * aberration * std::pow(wavelength, 3) * s2 * s2 +
		parameters[3] + std::asin(optics.get_amplitude_contrast())
	);
}

/**
 * @brief Insert the slices of a case as the contract states, in double.
 *
 * Every frequency of the full spectrum of each slice is placed, the voxels
 * with a negative x are left out, and the accumulators are the inputs plus
 * the insertions. The batches are K accumulators each receiving B slices,
 * with the transforms laid out (K, B) or (B) and every other input (B) or
 * without batch axes.
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

	const auto accumulator_count =
		data.get_numerator().size() / volume_count;
	const auto slice_count = data.get_slices().size() / (n * h);
	const auto transform_count = data.get_transforms().size() / 12;
	const auto radial_count = data.get_radial().size() / h;
	const auto variant = data.get_variant();
	const auto extra_size = variant == insertion_variant::ctf ? 4 : n * h;
	const auto extra_count = variant == insertion_variant::plain
		? 1
		: data.get_extra().size() / extra_size;

	complex_vector<double> numerator(data.get_numerator().size());
	std::vector<double> denominator(numerator.size());
	for (std::size_t i = 0; i < numerator.size(); ++i)
	{
		numerator[i] = data.get_numerator()[i];
		denominator[i] = data.get_denominator()[i];
	}

	const auto slices_per_accumulator = transform_count / accumulator_count;
	for (std::size_t k = 0; k < accumulator_count; ++k)
	for (std::size_t b = 0; b < slices_per_accumulator; ++b)
	{
		const auto *slice = &data.get_slices()[(b % slice_count) * n * h];
		const auto *transform = &data.get_transforms()[
			(k * slices_per_accumulator + b) * 12
		];
		const auto *radial = &data.get_radial()[(b % radial_count) * h];
		std::vector<double> extra(extra_size);
		for (std::size_t i = 0;
			variant != insertion_variant::plain && i < extra_size;
			++i)
		{
			extra[i] = data.get_extra()[(b % extra_count) * extra_size + i];
		}
		auto *num = &numerator[k * volume_count];
		auto *den = &denominator[k * volume_count];

		for (std::ptrdiff_t row = 0; row < signed_n; ++row)
		for (std::ptrdiff_t column = 0;
			column < static_cast<std::ptrdiff_t>(h);
			++column)
		{
			if (
				n % 2 == 0 &&
				(row == signed_n / 2 || column == signed_n / 2)
			)
			{
				continue;
			}
			const auto k_y = static_cast<double>(
				row <= signed_n / 2 ? row : row - signed_n
			);
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

			double alpha = 1.0;
			double beta = 1.0;
			if (variant == insertion_variant::weighted)
			{
				beta = extra[static_cast<std::size_t>(row) * h +
					static_cast<std::size_t>(column)];
			}
			else if (variant == insertion_variant::ctf)
			{
				alpha = reference_ctf(extra.data(), n, k_x, k_y);
				beta = alpha * alpha;
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
				std::array<double, 3> big_k;
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
					if (
						gx < 0 || gx > signed_big / 2 ||
						std::abs(gy) > signed_big / 2 ||
						std::abs(gz) > signed_big / 2
					)
					{
						continue;
					}
					const auto kappa =
						window_weight(window, gx - big_k[0]) *
						window_weight(window, gy - big_k[1]) *
						window_weight(window, gz - big_k[2]);
					const auto phase = std::polar(
						1.0,
						-2.0 * pi * (gx + gy + gz) * volume_centre /
							static_cast<double>(big)
					);
					const auto sy = gy < 0 ? gy + signed_big : gy;
					const auto sz = gz < 0 ? gz + signed_big : gz;
					const auto index =
						(sz * signed_big + sy) *
							static_cast<std::ptrdiff_t>(half) +
						gx;
					num[index] += kappa * r * alpha * value * phase;
					den[index] += kappa * r * beta;
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
bool identical(const insertion_result<T> &a, const insertion_result<T> &b)
{
	return
		a.get_numerator() == b.get_numerator() &&
		a.get_denominator() == b.get_denominator();
}

/**
 * @brief Three slices of extent n into one volume of extent big.
 */
template <typename T>
insertion_case<T> make_batch(
	insertion_variant variant,
	std::size_t n,
	std::size_t big,
	unsigned seed
)
{
	const auto h = n / 2 + 1;
	return insertion_case<T>(
		variant,
		case_extents(
			{ big, big, big / 2 + 1 },
			{ 3, n, h },
			{ 3, 3, 4 },
			{ 3, h },
			variant == insertion_variant::ctf
				? std::vector<std::size_t>{ 3, 4 }
				: std::vector<std::size_t>{ 3, n, h }
		),
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

inline
double tolerance_for(const em::gridding_window &window)
{
	return window.get_profile() == em::gridding_profile::kaiser_bessel
		? 5e-4
		: 1e-9;
}

/**
 * @brief The descriptors and buffers of the operands of a case, laid out
 * for the operation the case names.
 */
template <typename T>
class insertion_operands
{
public:
	/**
	 * @brief Lay out the operands of a case.
	 *
	 * @param data The case.
	 * @param in_place Whether the outputs are the accumulators themselves.
	 */
	insertion_operands(const insertion_case<T> &data, bool in_place)
		: m_count(count_elements(data.get_extents().get_accumulator()))
	{
		const auto &extents = data.get_extents();
		const auto numerator = make_buffer(data.get_numerator());
		const auto denominator = make_buffer(data.get_denominator());

		m_input_descriptors = {
			make_descriptor<T>(extents.get_accumulator(), true),
			make_descriptor<T>(extents.get_accumulator(), false),
			make_descriptor<T>(extents.get_slices(), true)
		};
		m_inputs = { numerator, denominator, make_buffer(data.get_slices()) };

		switch (data.get_variant())
		{
		case insertion_variant::plain:
			add(extents.get_transforms(), data.get_transforms());
			add(extents.get_radial(), data.get_radial());
			break;
		case insertion_variant::weighted:
			add(extents.get_extra(), data.get_extra());
			add(extents.get_transforms(), data.get_transforms());
			add(extents.get_radial(), data.get_radial());
			break;
		case insertion_variant::ctf:
			add(extents.get_transforms(), data.get_transforms());
			add(extents.get_extra(), data.get_extra());
			add(extents.get_radial(), data.get_radial());
			break;
		}

		m_output_descriptors = {
			make_descriptor<T>(extents.get_accumulator(), true),
			make_descriptor<T>(extents.get_accumulator(), false)
		};
		m_outputs = {
			in_place
				? numerator
				: make_buffer(complex_vector<T>(m_count)),
			in_place
				? denominator
				: make_buffer(std::vector<T>(m_count))
		};
	}

	const std::vector<operand_descriptor>& get_output_descriptors() const
	{
		return m_output_descriptors;
	}

	const std::vector<operand_descriptor>& get_input_descriptors() const
	{
		return m_input_descriptors;
	}

	/**
	 * @brief Run a program over the operands.
	 *
	 * @return insertion_result<T> The numerator and the denominator.
	 */
	insertion_result<T> execute(
		const cpu::cpu_program &program,
		cpu::thread_pool &pool
	) const
	{
		run(program, pool);
		return insertion_result<T>(
			read_buffer<std::complex<T>>(*m_outputs[0], m_count),
			read_buffer<T>(*m_outputs[1], m_count)
		);
	}

	/**
	 * @brief Run a program over the operands, without reading the outputs.
	 */
	void run(const cpu::cpu_program &program, cpu::thread_pool &pool) const
	{
		const std::vector<std::shared_ptr<buffer>> scratches;
		program.execute(
			make_span(m_outputs),
			make_span(m_inputs),
			make_span(scratches),
			pool
		);
	}

private:
	void add(
		const std::vector<std::size_t> &operand_extents,
		const std::vector<T> &values
	)
	{
		m_input_descriptors.push_back(
			make_descriptor<T>(operand_extents, false)
		);
		m_inputs.push_back(make_buffer(values));
	}

	std::size_t m_count;
	std::vector<operand_descriptor> m_output_descriptors;
	std::vector<operand_descriptor> m_input_descriptors;
	std::vector<std::shared_ptr<buffer>> m_outputs;
	std::vector<std::shared_ptr<const buffer>> m_inputs;
};

} // namespace test
} // namespace rexlib
