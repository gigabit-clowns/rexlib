// SPDX-License-Identifier: GPL-3.0-only

#include "test_arena.hpp"

#include <algorithm>
#include <new>

namespace xmipp4
{
namespace cuda
{

test_arena::test_arena(std::size_t capacity, std::size_t alignment) noexcept
	: m_capacity(capacity)
	, m_alignment(alignment)
	, m_used_bytes(0)
{
}

test_arena::~test_arena()
{
	for (const auto &item : m_regions)
	{
		release(item.first);
	}
}

void* test_arena::take(std::size_t size)
{
	if (size > m_capacity - m_used_bytes)
	{
		throw std::bad_alloc();
	}

	auto *result = ::operator new(size, std::align_val_t(m_alignment));
	m_regions.emplace_back(result, size);
	m_used_bytes += size;
	return result;
}

void test_arena::give_back(void *data) noexcept
{
	const auto ite = std::find_if(
		m_regions.begin(), m_regions.end(),
		[data] (const region_vector::value_type &item) noexcept -> bool
		{
			return item.first == data;
		}
	);

	if (ite == m_regions.end())
	{
		return;
	}

	m_used_bytes -= ite->second;
	m_regions.erase(ite);
	release(data);
}

std::size_t test_arena::get_alignment() const noexcept
{
	return m_alignment;
}

std::size_t test_arena::get_region_count() const noexcept
{
	return m_regions.size();
}

std::size_t test_arena::get_used_bytes() const noexcept
{
	return m_used_bytes;
}

void test_arena::release(void *data) noexcept
{
	::operator delete(data, std::align_val_t(m_alignment));
}

} // namespace cuda
} // namespace xmipp4
