// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <type_traits>
#include <cstddef>

namespace rex
{

/**
 * @brief Helper class to pass a heterogeneous sequence of types to a function.
 * 
 * This class offers similar functionality to `std::tuple`, but explicitly
 * avoids storing the actual values.
 * 
 * @tparam Ts Sequence of types.
 */
template <typename... Ts>
struct type_list {};

/**
 * @brief Utility class to obtain the size of a `type_list`.
 * 
 * @tparam T A specialization `type_list` for which the size is obtained.
 */
template <typename T>
struct type_list_size;

template <typename... Ts>
struct type_list_size<type_list<Ts...>>
	: std::integral_constant<std::size_t, sizeof...(Ts)>
{
};

/**
 * @brief Utility class to index a type on a `type_list`.
 * 
 * @tparam T A specialization `type_list` for which a type is extracted.
 */
template<std::size_t I, typename T>
struct type_list_element;

template<std::size_t I, typename Head, typename... Tail>
struct type_list_element<I, type_list<Head, Tail...>>
    : type_list_element<I - 1, type_list<Tail...>>
{
};
 
template<class Head, typename... Tail>
struct type_list_element<0, type_list<Head, Tail...>>
{
    using type = Head;
};

/**
 * @brief Utility class to prepend a type to the front of a `type_list`.
 *
 * @tparam T Type to be prepended.
 * @tparam List A specialization of `type_list` the type is prepended to.
 */
template <typename T, typename List>
struct type_list_prepend;

template <typename T, typename... Ts>
struct type_list_prepend<T, type_list<Ts...>>
{
	using type = type_list<T, Ts...>;
};

/**
 * @brief Utility class to obtain a `type_list` with a type repeated a fixed
 * number of times.
 *
 * @tparam T Type to be repeated.
 * @tparam N Number of repetitions.
 */
template <typename T, std::size_t N>
struct repeated_type_list
{
	using type = typename type_list_prepend<
		T,
		typename repeated_type_list<T, N - 1>::type
	>::type;
};

template <typename T>
struct repeated_type_list<T, 0>
{
	using type = type_list<>;
};

/**
 * @brief Utility class to concatenate an arbitrary number of `type_list`-s.
 *
 * @tparam Lists Sequence of `type_list` specializations to be concatenated.
 */
template <typename... Lists>
struct type_list_cat;

template <typename... Ts>
struct type_list_cat<type_list<Ts...>>
{
	using type = type_list<Ts...>;
};

template <typename... As, typename... Bs, typename... Rest>
struct type_list_cat<type_list<As...>, type_list<Bs...>, Rest...>
	: type_list_cat<type_list<As..., Bs...>, Rest...>
{
};

/**
 * @brief Convenience alias for `type_list_cat<Lists...>::type`.
 *
 * @tparam Lists Sequence of `type_list` specializations to be concatenated.
 */
template <typename... Lists>
using type_list_cat_t = typename type_list_cat<Lists...>::type;

} // namespace rex
