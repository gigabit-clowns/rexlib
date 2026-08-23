// SPDX-License-Identifier: GPL-3.0-only

#include "scalar_value.hpp"

#include "numerical_type_traits.hpp"
#include "numerical_type_dispatch.hpp"
#include "numerical_cast.hpp"

#include "../platform/attributes.hpp"

#include <new>
#include <utility>
#include <type_traits>
#include <typeinfo>

namespace rex
{

template <typename T>
inline
scalar_value::scalar_value(const T &value) noexcept
	: m_data_type(numerical_type_of<T>::value)
{
	static_assert(
		sizeof(T) <= storage_size,
		"T is too large to be stored in a scalar_value"
	);
	static_assert(
		alignof(T) <= storage_alignment,
		"T is over-aligned to be stored in a scalar_value"
	);

	// scalar_value relies on the defaulted copy, move and destructor to operate
	// on the raw storage, so T must be safe to copy and destroy as raw bytes.
	// Trivial copyability also entails trivial (byte-wise) destruction.
	static_assert(
		std::is_trivially_copyable<T>::value,
		"T must be trivially copyable to be stored in a scalar_value"
	);

	::new (static_cast<void*>(m_storage.data())) T(value);
}

template <typename T>
inline
const T* scalar_value::get_if() const noexcept
{
	if (numerical_type_of<T>::value != m_data_type)
	{
		return nullptr;
	}

	return reinterpret_cast<const T*>(m_storage.data());
}

template <typename T>
inline
const T& scalar_value::get() const
{
	const T *result = get_if<T>();
	if (result == nullptr)
	{
		throw bad_scalar_value_access();
	}

	return *result;
}

inline
numerical_type scalar_value::get_data_type() const noexcept
{
	return m_data_type;
}

template <typename Func>
inline
auto visit(Func &&func, const scalar_value &value)
{
	return dispatch_numerical_types(
		[&func, &value] (auto tag) -> decltype(auto)
		{
			using value_type = typename decltype(tag)::type;
			return std::forward<Func>(func)(value.get<value_type>());
		},
		value.get_data_type()
	);
}

namespace detail
{

template <typename T, typename Q>
inline
typename std::enable_if<std::is_constructible<T, Q>::value, T>::type
scalar_value_cast_impl(const Q &value)
{
	return numerical_cast<T>(value);
}

template <typename T, typename Q>
REX_NORETURN
inline
typename std::enable_if<!std::is_constructible<T, Q>::value, T>::type
scalar_value_cast_impl(const Q&)
{
	throw std::bad_cast();
}

} // namespace detail

template <typename T>
inline
T scalar_value_cast(const scalar_value &value)
{
	return visit(
		[] (const auto &stored) -> T
		{
			return detail::scalar_value_cast_impl<T>(stored);
		},
		value
	);
}

} // namespace rex
