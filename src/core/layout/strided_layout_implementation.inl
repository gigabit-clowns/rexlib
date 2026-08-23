// SPDX-License-Identifier: GPL-3.0-only

#include "strided_layout_implementation.hpp"

#include <rex/core/platform/assert.hpp>

#include <algorithm>
#include <numeric>

#include <boost/functional/hash.hpp>

namespace rex 
{

template <typename ForwardIt>
inline
void check_axis_permutation(ForwardIt first, ForwardIt last, std::size_t count)
{
    // Based on:
    // https://en.cppreference.com/w/cpp/algorithm/is_permutation
    // The former function could be re-used, provided that something
    // like boost::counting_iterator is available.

    if (std::distance(first, last) != static_cast<std::ptrdiff_t>(count))
    {
		throw std::invalid_argument(
			"Axis permutation's length does not match the required count"
		);
    }

    // Skip common prefix
    std::size_t i = 0;
    while (first != last && *first == i)
    {
		++first;
		++i;
    }

    // For the rest, check that it is a permutation
    while (i < count)
    {
		// Ensure the current value appears in the range.
		auto ite = std::find(first, last, i);
		if (ite == last)
		{
			std::ostringstream oss;
			oss << "Index " << i << " is missing in the axis permutation";
			throw std::invalid_argument(oss.str());
		}

		++i;
    }
}

class apply_subscripts_helper
{
public:
    static
    strided_layout_implementation::strided_axis_vector_type
    process(
		const dynamic_subscript *first_subscript, 
		const dynamic_subscript *last_subscript,
		const strided_axis *first_axis, 
		const strided_axis *last_axis,
		std::ptrdiff_t &offset 
    )
    {
		strided_layout_implementation::strided_axis_vector_type axes;

		process_forwards(
			first_subscript,
			last_subscript,
			first_axis,
			last_axis,
			offset,
			axes,
			axes.begin()
		);

		return axes;
    }

private:
    static
    void process_forwards(
		const dynamic_subscript *first_subscript, 
		const dynamic_subscript *last_subscript,
		const strided_axis *first_axis, 
		const strided_axis *last_axis,
		std::ptrdiff_t &offset,
		strided_layout_implementation::strided_axis_vector_type &axes,
		strided_layout_implementation::strided_axis_vector_type::iterator head_ite 
    )
    {
		while (first_subscript != last_subscript)
		{
			const auto &subscript = *first_subscript;

			switch (subscript.get_subscript_type())
			{
			case dynamic_subscript::subscript_type::ellipsis:
				process_backwards(
					std::next(first_subscript), last_subscript,
					first_axis, last_axis,
					offset,
					axes,
					head_ite
				);
				first_subscript = last_subscript; // Finishes processing
				first_axis = last_axis;
				break;

			case dynamic_subscript::subscript_type::new_axis:
				head_ite = axes.insert(head_ite, make_phantom_axis());
				++first_subscript;
				++head_ite;
				break;

			case dynamic_subscript::subscript_type::index:
				check_non_empty_axes_for_index(first_axis, last_axis);
				apply_index(*first_axis, offset, subscript.get_index());
				++first_subscript;
				++first_axis;
				break;
			
			case dynamic_subscript::subscript_type::slice:
				check_non_empty_axes_for_slice(first_axis, last_axis);
				head_ite = axes.insert(head_ite, *first_axis);
				apply_slice(*head_ite, offset, subscript.get_slice());
				++first_subscript;
				++first_axis;
				++head_ite;
				break;

			default:
				throw std::invalid_argument(
					"Unknown subscript type encountered"
				);
			}
		}
		
		// Copy reminder
		if (first_axis != last_axis)
		{
			axes.insert(head_ite, first_axis, last_axis);
		}
    }

    static
    void process_backwards(
		const dynamic_subscript *first_subscript, 
		const dynamic_subscript *last_subscript,
		const strided_axis *first_axis, 
		const strided_axis *last_axis,
		std::ptrdiff_t &offset,
		strided_layout_implementation::strided_axis_vector_type &axes,
		strided_layout_implementation::strided_axis_vector_type::iterator head_ite 
    )
    {
		while (first_subscript != last_subscript)
		{
			const auto &subscript = *std::prev(last_subscript);

			switch (subscript.get_subscript_type())
			{
			case dynamic_subscript::subscript_type::ellipsis:
				throw std::invalid_argument(
					"Two ellipsis tags were encountered when processing "
					"subscripts"
				);

			case dynamic_subscript::subscript_type::new_axis:
				head_ite = axes.insert(head_ite, make_phantom_axis());
				--last_subscript;
				break;

			case dynamic_subscript::subscript_type::index:
				check_non_empty_axes_for_index(first_axis, last_axis);
				apply_index(*std::prev(last_axis), offset, subscript.get_index());
				--last_subscript;
				--last_axis;
				break;
			
			case dynamic_subscript::subscript_type::slice:
				check_non_empty_axes_for_slice(first_axis, last_axis);
				head_ite = axes.insert(head_ite, *std::prev(last_axis));
				apply_slice(*head_ite, offset, subscript.get_slice());
				--last_subscript;
				--last_axis;
				break;

			default:
				throw std::invalid_argument(
					"Unknown subscript type encountered"
				);
			}
		}

		// Copy reminder
		if (first_axis != last_axis)
		{
			axes.insert(head_ite, first_axis, last_axis);
		}
    }

    static
    void check_non_empty_axes(
		const strided_axis *first_axis, 
		const strided_axis *last_axis,
		const char *subscript_type
    )
    {
		if (first_axis == last_axis)
		{
			std::ostringstream oss;
			oss << subscript_type 
			<< " subscript was encountered, but there are no more axes to process";
			throw std::invalid_argument(oss.str());
		}
    }

    static
    void check_non_empty_axes_for_slice(
		const strided_axis *first_axis, 
		const strided_axis *last_axis
    )
    {
		check_non_empty_axes(first_axis, last_axis, "A slice");
    }

    static
    void check_non_empty_axes_for_index(
		const strided_axis *first_axis, 
		const strided_axis *last_axis
    )
    {
		check_non_empty_axes(first_axis, last_axis, "An index");
    }
};


inline
strided_layout_implementation::strided_layout_implementation() noexcept
    : m_axes()
    , m_offset(0UL)
{
}

inline
strided_layout_implementation::strided_layout_implementation(
	const strided_axis_vector_type &axes,
	std::ptrdiff_t offset 
)
	: m_axes(axes)
	, m_offset(offset)
{
}

inline
strided_layout_implementation::strided_layout_implementation(
	strided_axis_vector_type &&axes,
	std::ptrdiff_t offset 
) noexcept
	: m_axes(std::move(axes))
	, m_offset(offset)
{
}

inline
std::size_t strided_layout_implementation::hash() const noexcept
{
	std::size_t seed = boost::hash_value(m_offset);
	for (const auto &axis : m_axes)
	{
		boost::hash_combine(seed, axis.hash());
	}
	return seed;
}

inline
std::size_t strided_layout_implementation::get_rank() const noexcept
{
	return m_axes.size();
}

template <typename Vec>
inline
void strided_layout_implementation::get_extents(
	Vec &extents
) const
{
	extents.clear();
	extents.reserve(m_axes.size());
	for (const auto &axis : m_axes)
	{
		extents.push_back(axis.get_extent());
	}
}

template <typename Vec>
inline
void strided_layout_implementation::get_strides(
	Vec &strides
) const
{
	strides.clear();
	strides.reserve(m_axes.size());
	for (const auto &axis : m_axes)
	{
		strides.push_back(axis.get_stride());
	}
}

inline
std::ptrdiff_t strided_layout_implementation::get_offset() const noexcept
{
	return m_offset;
}

inline
std::size_t 
strided_layout_implementation::compute_storage_requirement() const noexcept
{
	std::size_t result = 0;

	for (const auto &axis : m_axes)
	{
		const auto extent = axis.get_extent();
		if (extent == 0)
		{
			return 0;
		}

		const auto stride = axis.get_stride();
		if(stride > 0)
		{
			const std::size_t last_index = extent - 1;
			result += last_index*stride;
		}
	}

	return m_offset + result + 1;
}

inline
std::size_t 
strided_layout_implementation::compute_element_count() const noexcept
{
	return std::accumulate(
		m_axes.cbegin(), 
		m_axes.cend(),
		std::size_t(1),
		[] (std::size_t current, const strided_axis &axis)
		{
			return current * axis.get_extent();
		}
	);
}

inline
bool strided_layout_implementation::extents_equal(
	span<const std::size_t> extents
) const noexcept
{
	if (m_axes.size() != extents.size())
	{
		return false;
	}

	const auto n = m_axes.size();
	for (std::size_t i = 0; i < n; ++i)
	{
		if (m_axes[i].get_extent() != extents[i])
		{
			return false;
		}
	}

	return true;
}

inline
strided_layout_implementation
strided_layout_implementation::apply_subscripts(
	span<const dynamic_subscript> subscripts
) const
{
	std::ptrdiff_t offset = m_offset;
	auto axes = apply_subscripts_helper::process(
		subscripts.begin(), 
		subscripts.end(),
		m_axes.data(), 
		m_axes.data() + m_axes.size(),
		offset
	);

	return strided_layout_implementation(std::move(axes), offset);
}

inline
strided_layout_implementation strided_layout_implementation::transpose() const
{
	strided_axis_vector_type axes;
	axes.reserve(m_axes.size());

	std::reverse_copy(
		m_axes.cbegin(), 
		m_axes.cend(),
		std::back_inserter(axes)
	);

	return strided_layout_implementation(std::move(axes), m_offset);
}

inline
strided_layout_implementation 
strided_layout_implementation::permute(span<const std::size_t> order) const
{
	const auto count = m_axes.size();
	check_axis_permutation(order.begin(), order.end(), count);

	strided_axis_vector_type axes;
	axes.reserve(count);

	for(std::size_t i = 0; i < count; ++i)
	{
		axes.push_back(m_axes[order[i]]);
	}

	return strided_layout_implementation(std::move(axes), m_offset);
}

inline
strided_layout_implementation strided_layout_implementation::matrix_transpose(
	std::ptrdiff_t axis1, 
	std::ptrdiff_t axis2
) const
{
	const auto n = m_axes.size();
	const auto index1 = sanitize_index(axis1, n);
	const auto index2 = sanitize_index(axis2, n);

	auto axes = m_axes;
	std::swap(axes[index1], axes[index2]);

	return strided_layout_implementation(std::move(axes), m_offset);
}

inline
strided_layout_implementation strided_layout_implementation::matrix_diagonal(
	std::ptrdiff_t axis1, 
	std::ptrdiff_t axis2
) const
{
	const auto n = m_axes.size();
	auto index1 = sanitize_index(axis1, n);
	auto index2 = sanitize_index(axis2, n);
	if (axis1 == axis2)
	{
		throw std::invalid_argument(
			"axis1 and axis2 must represent different axes"
		);
	}

	if (index1 > index2)
	{
		std::swap(index1, index2); // Sort
	}

	strided_axis_vector_type axes;
	axes.reserve(n - 1);
	REX_ASSERT(index1 < index2);
	std::copy(
		m_axes.cbegin(),
		std::next(m_axes.cbegin(), index1),
		std::back_inserter(axes)
	);
	std::copy(
		std::next(m_axes.cbegin(), index1+1),
		std::next(m_axes.cbegin(), index2),
		std::back_inserter(axes)
	);
	std::copy(
		std::next(m_axes.cbegin(), index2+1),
		m_axes.cend(),
		std::back_inserter(axes)
	);
	REX_ASSERT(axes.size() == (n - 2));

	const auto &axis_a = m_axes[index1];
	const auto &axis_b = m_axes[index2];
	const auto extent = std::min(axis_a.get_extent(), axis_b.get_extent());
	const auto stride = axis_a.get_stride() + axis_b.get_stride();
	axes.emplace_back(extent, stride);
	
	REX_ASSERT(axes.size() == (n - 1));
	return strided_layout_implementation(std::move(axes), m_offset);
}

inline
strided_layout_implementation strided_layout_implementation::squeeze() const
{
	strided_axis_vector_type axes;

	std::copy_if(
		m_axes.cbegin(), m_axes.cend(),
		std::back_inserter(axes),
		is_significant
	);

	return strided_layout_implementation(std::move(axes), m_offset);
}

inline
strided_layout_implementation strided_layout_implementation::broadcast_to(
	span<const std::size_t> extents
) const
{
	if (m_axes.size() > extents.size())
	{
		std::ostringstream oss;
		oss << "Cannot broadcast layout with " << m_axes.size()
			<< " axes into a shape of " << extents.size()
			<< " dimensions.";
		throw std::invalid_argument(oss.str());
	}

	strided_axis_vector_type axes;
	axes.reserve(extents.size());
	
	const std::size_t padding = extents.size() - m_axes.size();
	std::fill_n(
		std::back_inserter(axes), padding,
		make_phantom_axis()
	);
	std::copy(
		m_axes.cbegin(), m_axes.cend(),
		std::back_inserter(axes)
	);

	const auto count = extents.size();
	for(std::size_t i = 0; i < count; ++i)
	{
		auto &axis = axes[i];
		const auto extent = extents[i];
		if (!rex::broadcast_to(axis, extent))
		{
			std::ostringstream oss;
			oss << "Cannot broadcast axis of extent " << axis.get_extent()
				<< " into an extent of " << extent << ".";
			throw std::invalid_argument(oss.str());
		}
	}

	return strided_layout_implementation(std::move(axes), m_offset);
}

} // namespace rex
