// SPDX-License-Identifier: GPL-3.0-only

#include <backends/cpu/plans/named_core_frame.hpp>

#include <numeric>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace rexlib
{
namespace cpu
{

namespace
{

template <typename T>
std::vector<T> select_axes(
	span<const T> values,
	const std::vector<std::size_t> &axes
)
{
	std::vector<T> result;
	result.reserve(axes.size());
	for (const auto axis : axes)
	{
		result.push_back(values[axis]);
	}
	return result;
}

} // anonymous namespace

named_core_frame::named_core_frame(
	std::vector<std::size_t> frame_extents,
	span<const std::size_t> named_axes
)
	: m_frame_extents(std::move(frame_extents))
{
	const auto rank = m_frame_extents.size();

	std::vector<bool> named(rank, false);
	for (const auto axis : named_axes)
	{
		if (axis >= rank)
		{
			std::ostringstream oss;
			oss << "named_core_frame: Axis " << axis
				<< " is out of range for a rank of " << rank << ".";
			throw std::out_of_range(oss.str());
		}
		if (named[axis])
		{
			std::ostringstream oss;
			oss << "named_core_frame: Axis " << axis
				<< " is named more than once.";
			throw std::invalid_argument(oss.str());
		}
		named[axis] = true;
	}

	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		(named[axis] ? m_core_axes : m_batch_axes).push_back(axis);
	}
}

span<const std::size_t> named_core_frame::get_frame_extents() const noexcept
{
	return make_span(m_frame_extents);
}

span<const std::size_t> named_core_frame::get_core_axes() const noexcept
{
	return make_span(m_core_axes);
}

span<const std::size_t> named_core_frame::get_batch_axes() const noexcept
{
	return make_span(m_batch_axes);
}

std::vector<std::size_t> named_core_frame::get_core_extents() const
{
	return select_axes(make_span(m_frame_extents), m_core_axes);
}

std::vector<std::size_t> named_core_frame::get_batch_extents() const
{
	return select_axes(make_span(m_frame_extents), m_batch_axes);
}

strided_layout named_core_frame::embed(
	const strided_layout &layout,
	bool carries_core
) const
{
	const auto rank = layout.get_rank();

	std::vector<std::size_t> positions;
	if (carries_core)
	{
		positions.resize(m_frame_extents.size());
		std::iota(positions.begin(), positions.end(), std::size_t(0));
	}
	else
	{
		positions = m_batch_axes;
	}

	if (rank > positions.size())
	{
		std::ostringstream oss;
		oss << "named_core_frame: An operand "
			<< (carries_core ? "carrying" : "without")
			<< " core axes has " << rank << " axes, but it can be placed in "
			<< "only " << positions.size() << ".";
		throw std::invalid_argument(oss.str());
	}

	std::vector<std::size_t> extents;
	std::vector<std::ptrdiff_t> strides;
	layout.get_extents(extents);
	layout.get_strides(strides);

	std::vector<std::size_t> frame_extents(m_frame_extents.size(), 1);
	std::vector<std::ptrdiff_t> frame_strides(m_frame_extents.size(), 0);
	const auto first = positions.size() - rank;
	for (std::size_t axis = 0; axis < rank; ++axis)
	{
		const auto position = positions[first + axis];
		frame_extents[position] = extents[axis];
		frame_strides[position] = strides[axis];
	}

	return strided_layout::make_custom_layout(
		make_span(frame_extents),
		make_span(frame_strides),
		layout.get_offset()
	);
}

strided_layout named_core_frame::align(
	const strided_layout &layout,
	bool carries_core
) const
{
	return embed(layout, carries_core).broadcast_to(
		make_span(m_frame_extents)
	);
}

std::vector<std::ptrdiff_t> named_core_frame::select_core(
	span<const std::ptrdiff_t> frame_values
) const
{
	return select_axes(frame_values, m_core_axes);
}

std::vector<std::ptrdiff_t> named_core_frame::select_batch(
	span<const std::ptrdiff_t> frame_values
) const
{
	return select_axes(frame_values, m_batch_axes);
}

} // namespace cpu
} // namespace rexlib
