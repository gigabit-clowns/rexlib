// SPDX-License-Identifier: GPL-3.0-only

#include "joint_layout_implementation.hpp"

#include <xmipp4/core/platform/assert.hpp>

#include <numeric>
#include <algorithm>
#include <sstream>

/**
 * Some of the algorithms and data structured featured in this code are based 
 * on:
 * https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/TensorIterator.cpp
 * 
 */

namespace xmipp4 
{

inline
joint_layout_implementation::
joint_layout_implementation(const extent_vector_type &extents)
	: m_extents(extents)
{
}

inline
void joint_layout_implementation::add_operand(
	const stride_vector_type &strides,
	std::ptrdiff_t offset
)
{
	XMIPP4_ASSERT( m_extents.size() == strides.size() );
	m_operands.emplace_back(
		strides,
		offset
	);
}

inline
void joint_layout_implementation::add_operand(
	stride_vector_type &&strides,
	std::ptrdiff_t offset
)
{
	XMIPP4_ASSERT( m_extents.size() == strides.size() );
	m_operands.emplace_back(
		std::move(strides),
		offset
	);
}

inline
const joint_layout_operand& 
joint_layout_implementation::get_operand(std::size_t index) const
{
	if (index >= m_operands.size())
	{
		throw std::out_of_range("Operand index is out of range");
	}

	return m_operands[index];
}

inline
void joint_layout_implementation::insert_largest_stride(
	span<std::size_t> permutation,
	std::size_t i
)
{
	const auto index1 = i;
	for (std::size_t j = 1; j <= i; ++j)
	{
		const auto index0 = i - j;
		const auto comparison = compare_strides(
			permutation[index0], 
			permutation[index1]
		);

		if (comparison > 0)
		{
			std::swap(permutation[index0], permutation[index1]);
			j = 0;
		} 
		else if (comparison < 0) 
		{
			break;
		}
	}
}

inline
void joint_layout_implementation::sort_axes_by_locality()
{
	const auto n = m_extents.size();
	if (n <= 1)
	{
		return; // Trivial
	}

	// Start with reversed indices n-1, n-2 ... 1, 0
	boost::container::small_vector<
		std::size_t, XMIPP4_SMALL_AXIS_COUNT
	> permutation(n);
	const auto permutation_view = span<std::size_t>(permutation.data(), n);
	std::generate(
		permutation.begin(), 
		permutation.end(), 
		[n = n]() mutable { return --n; }
	);

	// Insertion sort with support for ambiguous comparisons
	for (std::size_t i = 1; i < n; ++i)
	{
		insert_largest_stride(permutation_view, i);
	}

	permute_axes(permutation_view);
}

inline
void joint_layout_implementation::coalesce_contiguous_axes()
{
	const auto n = m_extents.size();
	if (n <= 1)
	{
		return; // Trivial
	}

	std::size_t prev = 0;
	for (std::size_t curr = 1; curr < n;  ++curr)
	{
		if (!try_coalesce_axes(prev, curr))
		{
			++prev;
			if(prev != curr) // Avoid unnecessary swaps.
			{
				swap_axes(prev, curr);
			}
		}
	}

	trim_axes(prev+1);
}

inline
std::size_t 
joint_layout_implementation::get_rank() const noexcept
{
	return m_extents.size();
}

inline
std::size_t 
joint_layout_implementation::get_number_of_operands(
) const noexcept
{
	return m_operands.size();
}

inline
span<const std::size_t> 
joint_layout_implementation::get_extents() const noexcept
{
	return span<const std::size_t>(m_extents.data(), m_extents.size());
}

inline
span<const std::ptrdiff_t> 
joint_layout_implementation::get_strides(std::size_t operand) const
{
	return get_operand(operand).get_strides();
}

inline
std::ptrdiff_t 
joint_layout_implementation::get_offset(std::size_t operand) const
{
	return get_operand(operand).get_offset();
}

inline
std::size_t joint_layout_implementation::iter(
	joint_cursor &ite,
	std::size_t first_dim,
	std::size_t last_dim
) const
{
	const auto valid = std::all_of(
		m_extents.cbegin(), 
		m_extents.cend(),
		[] (auto extent)
		{
			return extent > 0;
		}
	);

	if (!valid)
	{
		// There is at least 1 empty axis
		return 0UL;
	}

	std::vector<std::ptrdiff_t> offsets;
	offsets.reserve(m_operands.size());
	std::transform(
		m_operands.cbegin(),
		m_operands.cend(),
		std::back_inserter(offsets),
		std::mem_fn(&joint_layout_operand::get_offset)
	);

	ite =  joint_cursor(
		m_extents.size(),
		std::move(offsets)
	);

	last_dim = std::min(last_dim, m_extents.size());
	if (first_dim >= last_dim)
	{
		return 1UL;
	}

	return m_extents[first_dim];
}

inline
std::size_t
joint_layout_implementation::compute_element_count() const noexcept
{
	std::size_t result = 1;
	for (const auto extent : m_extents)
	{
		result *= extent;
	}
	return result;
}

inline
std::size_t joint_layout_implementation::seek(
	joint_cursor &ite,
	std::size_t position,
	std::size_t first_dim,
	std::size_t last_dim
) const
{
	const auto run = iter(ite, first_dim, last_dim);
	if (run == 0 || position == 0)
	{
		// Either there is nothing to iterate, or iter() has already left the
		// cursor exactly where it was asked for.
		return run;
	}

	last_dim = std::min(last_dim, m_extents.size());
	XMIPP4_ASSERT( first_dim < last_dim );

	const auto indices = ite.get_indices();
	const auto offsets = ite.get_offsets();

	// Decomposing the position the same way the traversal composes it: the
	// inner-most axis varies fastest, so it takes the remainder and passes
	// the quotient outwards. The quotient runs out once every remaining index
	// would be zero, which is what iter() has already left behind, so the
	// loop stops rather than writing them.
	for (std::size_t i = first_dim; i < last_dim; ++i)
	{
		if (position == 0)
		{
			break;
		}

		const auto index = position % m_extents[i];
		position /= m_extents[i];

		indices[i] = index;
		apply_strides(offsets, i, static_cast<std::ptrdiff_t>(index));
	}

	// Anything left over is a position past the end of the iterated range.
	XMIPP4_ASSERT( position == 0 );

	return m_extents[first_dim] - indices[first_dim];
}

inline
std::size_t joint_layout_implementation::next(
	joint_cursor &ite,
	std::size_t n,
	std::size_t first_dim,
	std::size_t last_dim
) const noexcept
{
	const auto extents = get_extents();
	const auto n_dim = extents.size();
	last_dim = std::min(last_dim, n_dim);

	if (first_dim >= last_dim)
	{
		// No "outer" loop to process.
		return 0;
	}
	
	const auto indices = ite.get_indices();
	XMIPP4_ASSERT( indices.size() == n_dim );

	if (n == 0)
	{
		// Fast return for query.
		return extents[first_dim] - indices[first_dim];
	}

	const auto offsets = ite.get_offsets();

	// Advance the inner axis by n - 1, so that after the loop it is incremented
	// by n
	if (n > 1)
	{
		const auto base_increment = n - 1;
		apply_strides(offsets, first_dim, base_increment);
		indices[first_dim] += base_increment;
	}

	for (std::size_t i = first_dim; i < last_dim; ++i) 
	{
		const auto next_index = indices[i] + 1;
		const auto extent = extents[i];

		if (next_index < extent)
		{
			apply_strides(offsets, i, 1);
			indices[i] = next_index;

			return extents[first_dim] - indices[first_dim];
		}

		apply_strides(offsets, i, -static_cast<std::ptrdiff_t>(indices[i]));
		indices[i] = 0;
	}
	
	return 0UL; 
}

inline
int joint_layout_implementation::compare_strides(
	std::size_t i, 
	std::size_t j
) noexcept
{
	for (const auto &operand : m_operands)
	{
		const auto cmp = operand.compare_strides(i, j);
		if (cmp != 0)
		{
			return cmp;
		}
	}

	// Untie with extents
	return static_cast<int>(m_extents[i]) - static_cast<int>(m_extents[j]); 
}

inline
void joint_layout_implementation::swap_axes(
	std::size_t i, 
	std::size_t j
) noexcept
{
	std::swap(m_extents[i], m_extents[j]);
	for (auto &operand : m_operands)
	{
		operand.swap_axes(i, j);
	}
}

inline
void joint_layout_implementation::permute_axes(
	span<std::size_t> permutation
)
{
	// Permute the extents using cycle decomposition
	const auto n = permutation.size();
	for (size_t i = 0; i < n; ++i) {
		while (permutation[i] != i) {
			swap_axes(i, permutation[i]);
			std::swap(permutation[i], permutation[permutation[i]]);
		}
	}
}

inline
bool joint_layout_implementation::try_coalesce_axes(
	std::size_t i, 
	std::size_t j
)
{
	if (m_extents[i] == 1)
	{
		swap_axes(i, j);
		return true;
	}

	if (can_coalesce_axes(i, j))
	{
		m_extents[i] *= m_extents[j];
		return true;
	}

	return false;
}

inline
bool joint_layout_implementation::can_coalesce_axes(
	std::size_t i, 
	std::size_t j
)
{
	const auto extent_i = m_extents[i];
	const auto extent_j = m_extents[j];
	if (extent_j == 1)
	{
		return true; // Trivial, as any stride_j is meaningless.
	}

	for (const auto &operand : m_operands)
	{
		const auto strides = operand.get_strides();
		const auto stride_i = strides[i];
		const auto stride_j = strides[j];
		if (static_cast<std::ptrdiff_t>(extent_i) * stride_i != stride_j) 
		{
			return false;
		}
	}

	return true;
}

inline
void joint_layout_implementation::trim_axes(std::size_t n)
{
	m_extents.resize(n);
	for (auto &operand : m_operands)
	{
		operand.trim_axes(n);
	}
}

inline
void joint_layout_implementation::apply_strides(
	span<std::ptrdiff_t> offsets, 
	std::size_t position, 
	std::ptrdiff_t multiplier
) const noexcept
{
	const auto n_operands = offsets.size();
	XMIPP4_ASSERT( m_operands.size() == n_operands );
	for (std::size_t j = 0; j < n_operands; ++j) 
	{
		const auto strides = m_operands[j].get_strides();
		offsets[j] += multiplier*strides[position];
	}
}

} // namespace xmipp4
