// SPDX-License-Identifier: GPL-3.0-only

#include "span.hpp"

#include "platform/assert.hpp"

#include <unordered_map>
#include <type_traits>

namespace rexlib
{

namespace detail
{

template <typename T, std::size_t S>
REXLIB_INLINE_CONSTEXPR
span_storage<T, S>::span_storage(T* ptr, std::size_t) noexcept
	: m_data(ptr)
{
}

template <typename T, std::size_t S>
REXLIB_INLINE_CONSTEXPR
T* span_storage<T, S>::get_data() const noexcept
{
	return m_data;
}

template <typename T, std::size_t S>
REXLIB_INLINE_CONSTEXPR
std::size_t span_storage<T, S>::get_size() const noexcept
{
	return S;
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span_storage<T, dynamic_extent>::span_storage() noexcept
	: m_data(nullptr)
	, m_size(0)
{
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span_storage<T, dynamic_extent>::span_storage(
	T* ptr,
	std::size_t count
) noexcept
	: m_data(ptr)
	, m_size(count)
{
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
T* span_storage<T, dynamic_extent>::get_data() const noexcept
{
	return m_data;
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
std::size_t span_storage<T, dynamic_extent>::get_size() const noexcept
{
	return m_size;
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span_storage<T, 0>::span_storage(T*, std::size_t) noexcept
{
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
T* span_storage<T, 0>::get_data() const noexcept
{
	return nullptr;
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
std::size_t span_storage<T, 0>::get_size() const noexcept
{
	return 0;
}

} // namespace detail

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR span<T, Extent>::span(pointer ptr, size_type count)
	: m_storage(ptr, count)
{
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR span<T, Extent>::span(pointer first, pointer last)
	: span(first, std::distance(first, last))
{
}

template <typename T, std::size_t Extent>
template <std::size_t Extent2>
REXLIB_INLINE_CONSTEXPR span<T, Extent>
::span(const span<value_type, Extent2> &other)
	: span(other.data(), other.size())
{
}

template <typename T, std::size_t Extent>
template <std::size_t Extent2>
REXLIB_INLINE_CONSTEXPR span<T, Extent>
::span(const span<const value_type, Extent2> &other)
	: span(other.data(), other.size())
{
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::size_type span<T, Extent>::size() const noexcept
{
	return m_storage.get_size();
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::size_type span<T, Extent>::size_bytes() const noexcept
{
	return size() * sizeof(element_type);
}

template <typename T, std::size_t Extent>
REXLIB_NODISCARD REXLIB_INLINE_CONSTEXPR
bool span<T, Extent>::empty() const noexcept
{
	return size() == 0;
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::reference
span<T, Extent>::operator[](size_type idx) const noexcept
{
	return data()[idx];
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::reference span<T, Extent>::front() const noexcept
{
	return *data();
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::reference span<T, Extent>::back() const noexcept
{
	return *(data() + size() - 1);
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::pointer span<T, Extent>::data() const noexcept
{
	return m_storage.get_data();
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::iterator span<T, Extent>::begin() const noexcept
{
	return data();
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::iterator span<T, Extent>::end() const noexcept
{
	return data() + size();
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::reverse_iterator
span<T, Extent>::rbegin() const noexcept
{
	return reverse_iterator(end());
}

template <typename T, std::size_t Extent>
REXLIB_INLINE_CONSTEXPR
typename span<T, Extent>::reverse_iterator
span<T, Extent>::rend() const noexcept
{
	return reverse_iterator(begin());
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span<byte> as_bytes(const span<T> &other) noexcept
{
	return span<byte>(
		as_bytes(other.data()),
		other.size_bytes()
	);
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span<const byte> as_bytes(const span<const T> &other) noexcept
{
	return span<const byte>(
		as_bytes(other.data()),
		other.size_bytes()
	);
}

template <typename T, std::size_t N>
REXLIB_INLINE_CONSTEXPR
span<byte, N*sizeof(T)> as_bytes(const span<T, N> &other) noexcept
{
	return span<byte, N*sizeof(T)>(
		as_bytes(other.data()),
		other.size_bytes()
	);
}

template <typename T, std::size_t N>
REXLIB_INLINE_CONSTEXPR
span<const byte, N*sizeof(T)> as_bytes(const span<const T, N> &other) noexcept
{
	return span<const byte, N*sizeof(T)>(
		as_bytes(other.data()),
		other.size_bytes()
	);
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span<T> make_span(T* ptr, std::size_t count) noexcept
{
	return span<T>(ptr, count);
}

template <typename T>
REXLIB_INLINE_CONSTEXPR
span<const T> make_span(const T* ptr, std::size_t count) noexcept
{
	return span<const T>(ptr, count);
}

template <typename T, std::size_t N>
REXLIB_INLINE_CONSTEXPR span<T, N> make_span(std::array<T, N> &array) noexcept
{
	return span<T, N>(array.data(), array.size());
}

template <typename T, std::size_t N>
REXLIB_INLINE_CONSTEXPR span<const T, N>
make_span(const std::array<T, N> &array) noexcept
{
	return span<const T, N>(array.data(), array.size());
}

template <typename T>
REXLIB_INLINE_CONSTEXPR span<T> make_span(std::vector<T> &array) noexcept
{
	return span<T>(array.data(), array.size());
}

template <typename T>
REXLIB_INLINE_CONSTEXPR span<const T>
make_span(const std::vector<T> &array) noexcept
{
	return span<const T>(array.data(), array.size());
}

} // namespace rexlib
