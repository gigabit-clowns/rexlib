// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/fourier_layout_plan.hpp>

#include <rexlib/core/dispatch/operand_signature.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/layout/strided_layout.hpp>
#include <rexlib/core/numerical/numerical_type.hpp>

#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace cpu
{

namespace
{

/**
 * @brief Restate a stride in the unit pocketfft measures them in.
 *
 * A layout counts in elements and pocketfft counts in bytes, which is what
 * lets it transform an operand whose elements are interleaved with something
 * else. The element type is known once the program is built, so the
 * conversion happens here and never again.
 */
std::vector<std::ptrdiff_t> to_byte_strides(
	const std::vector<std::ptrdiff_t> &strides,
	std::size_t element_size
)
{
	std::vector<std::ptrdiff_t> result;
	result.reserve(strides.size());
	for (const auto stride : strides)
	{
		result.push_back(stride * static_cast<std::ptrdiff_t>(element_size));
	}
	return result;
}

void check_axes_within_rank(
	span<const std::size_t> axes,
	std::size_t rank
)
{
	for (const auto axis : axes)
	{
		if (axis >= rank)
		{
			std::ostringstream oss;
			oss << "fourier_layout_plan: Axis " << axis
				<< " is out of range for a rank of " << rank << ".";
			throw std::out_of_range(oss.str());
		}
	}
}

void check_equal_extents(
	const std::vector<std::size_t> &output_extents,
	const std::vector<std::size_t> &input_extents
)
{
	if (output_extents != input_extents)
	{
		throw std::invalid_argument(
			"fourier_layout_plan: A transform that neither stores nor reads a "
			"half spectrum leaves the extents as it found them, but the two "
			"operands do not have the same ones."
		);
	}
}

/**
 * @brief Check a half spectrum against the signal it belongs to.
 *
 * Only the last transformed axis is halved. The symmetry of a real signal's
 * spectrum relates a coefficient to the one at the negated index along every
 * transformed axis at once, so it can only be exploited once.
 */
void check_spectrum_extents(
	const std::vector<std::size_t> &signal_extents,
	const std::vector<std::size_t> &spectrum_extents,
	std::size_t halved_axis
)
{
	if (signal_extents.size() != spectrum_extents.size())
	{
		throw std::invalid_argument(
			"fourier_layout_plan: A half spectrum has the rank of the signal "
			"it comes from, but the two operands do not have the same one."
		);
	}

	for (std::size_t i = 0; i < signal_extents.size(); ++i)
	{
		const auto expected = (i == halved_axis)
			? (signal_extents[i] / 2) + 1
			: signal_extents[i];

		if (spectrum_extents[i] != expected)
		{
			std::ostringstream oss;
			oss << "fourier_layout_plan: An extent of " << expected
				<< " was expected along axis " << i << " of the half "
				<< "spectrum, but it has " << spectrum_extents[i] << ".";
			throw std::invalid_argument(oss.str());
		}
	}
}

/**
 * @brief Count the samples reaching each transformed value.
 *
 * An axis with no sample along it has no transform: there is nothing to read
 * and, a half spectrum of an empty axis still holding one coefficient,
 * nothing that could be written either. It is turned away here rather than
 * left to leave an output unwritten, which is what NumPy does with it too.
 */
std::size_t count_samples(
	const std::vector<std::size_t> &shape,
	span<const std::size_t> axes
)
{
	std::size_t result = 1;
	for (const auto axis : axes)
	{
		if (shape[axis] == 0)
		{
			std::ostringstream oss;
			oss << "fourier_layout_plan: Axis " << axis
				<< " is empty, so there is no transform to perform along it.";
			throw std::invalid_argument(oss.str());
		}
		result *= shape[axis];
	}
	return result;
}

} // anonymous namespace

fourier_layout_plan::fourier_layout_plan(
	const operand_signature &output_signature,
	const operand_signature &input_signature,
	span<const std::size_t> axes,
	fourier_transform_kind kind
)
	: m_axes(axes.begin(), axes.end())
	, m_input_offset(input_signature.get_layout().get_offset())
	, m_output_offset(output_signature.get_layout().get_offset())
	, m_sample_count(1)
{
	const auto &output_layout = output_signature.get_layout();
	const auto &input_layout = input_signature.get_layout();

	std::vector<std::size_t> output_extents;
	std::vector<std::size_t> input_extents;
	std::vector<std::ptrdiff_t> output_strides;
	std::vector<std::ptrdiff_t> input_strides;
	output_layout.get_extents(output_extents);
	input_layout.get_extents(input_extents);
	output_layout.get_strides(output_strides);
	input_layout.get_strides(input_strides);

	if (kind != fourier_transform_kind::complex_to_complex && axes.empty())
	{
		throw std::invalid_argument(
			"fourier_layout_plan: A transform between a real operand and a "
			"half spectrum needs an axis to halve."
		);
	}

	// The signal is what the shape is taken from, so the rank the axes are
	// checked against is settled before they are used to index into it.
	check_axes_within_rank(axes, input_extents.size());

	switch (kind)
	{
	case fourier_transform_kind::complex_to_complex:
		check_equal_extents(output_extents, input_extents);
		m_shape = input_extents;
		break;

	case fourier_transform_kind::real_to_complex:
		check_spectrum_extents(input_extents, output_extents, axes.back());
		m_shape = input_extents;
		break;

	case fourier_transform_kind::complex_to_real:
		check_axes_within_rank(axes, output_extents.size());
		check_spectrum_extents(output_extents, input_extents, axes.back());
		m_shape = output_extents;
		break;
	}

	m_input_strides = to_byte_strides(
		input_strides,
		get_size(input_signature.get_data_type())
	);
	m_output_strides = to_byte_strides(
		output_strides,
		get_size(output_signature.get_data_type())
	);
	m_sample_count = count_samples(m_shape, axes);

	// The two operands only pair up element by element when they share their
	// extents, which is the one kind that ever needs them paired.
	if (kind == fourier_transform_kind::complex_to_complex)
	{
		joint_layout_builder builder;
		builder.set_extents(make_span(m_shape));
		builder.add_operand(
			make_span(m_shape),
			make_span(output_strides),
			0
		);
		builder.add_operand(
			make_span(m_shape),
			make_span(input_strides),
			0
		);
		m_conversion_layout = builder.build();
	}
}

const std::vector<std::size_t>&
fourier_layout_plan::get_shape() const noexcept
{
	return m_shape;
}

const std::vector<std::size_t>&
fourier_layout_plan::get_axes() const noexcept
{
	return m_axes;
}

const std::vector<std::ptrdiff_t>&
fourier_layout_plan::get_input_strides() const noexcept
{
	return m_input_strides;
}

const std::vector<std::ptrdiff_t>&
fourier_layout_plan::get_output_strides() const noexcept
{
	return m_output_strides;
}

std::ptrdiff_t fourier_layout_plan::get_input_offset() const noexcept
{
	return m_input_offset;
}

std::ptrdiff_t fourier_layout_plan::get_output_offset() const noexcept
{
	return m_output_offset;
}

std::size_t fourier_layout_plan::get_sample_count() const noexcept
{
	return m_sample_count;
}

const joint_layout&
fourier_layout_plan::get_conversion_layout() const noexcept
{
	return m_conversion_layout;
}

} // namespace cpu
} // namespace rexlib
