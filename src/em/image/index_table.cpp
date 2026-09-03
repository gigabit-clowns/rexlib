// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/image/index_table.hpp>

#include <rexlib/core/platform/assert.hpp>

#include <stdexcept>

namespace rexlib
{
namespace em
{

index_table::index_table() noexcept
	: m_rank(0)
	, m_size(0)
{
}

index_table::index_table(std::size_t rank)
	: m_rank(rank)
	, m_size(0)
{
}

index_table::index_table(const index_table &other) = default;
index_table::index_table(index_table &&other) noexcept = default;
index_table::~index_table() = default;

index_table& index_table::operator=(const index_table &other) = default;
index_table& index_table::operator=(index_table &&other) noexcept = default;

void index_table::reset(std::size_t rank)
{
	m_rank = rank;
	clear();
}

void index_table::add(span<const std::size_t> index)
{
	if (index.size() != m_rank)
	{
		throw std::invalid_argument(
			"index_table::add: The index does not have the rank of the "
			"table."
		);
	}

	m_values.insert(m_values.end(), index.begin(), index.end());
	++m_size;
}

void index_table::clear() noexcept
{
	m_values.clear();
	m_size = 0;
}

void index_table::reserve(std::size_t count)
{
	m_values.reserve(count * m_rank);
}

std::size_t index_table::get_rank() const noexcept
{
	return m_rank;
}

std::size_t index_table::get_size() const noexcept
{
	return m_size;
}

span<const std::size_t> index_table::get(std::size_t position) const noexcept
{
	REXLIB_ASSERT(position < m_size);
	return make_span(m_values.data() + (position * m_rank), m_rank);
}

} // namespace em
} // namespace rexlib
