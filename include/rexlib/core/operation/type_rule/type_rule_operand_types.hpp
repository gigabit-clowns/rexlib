// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/meta/type_list.hpp>

#include <cstddef>

namespace rexlib
{

/**
 * @brief Apply the transform of every slot to the type of its pivot.
 *
 * This is the type level counterpart of the runtime slot evaluation. It
 * yields the element type of every operand from the resolved pivot types.
 *
 * @tparam Slots type_list of type_slot, one per operand.
 * @tparam Pivots type_list of the resolved pivot element types.
 *
 * @see operand_type_rule
 * @see type_slot
 */
template <typename Slots, typename Pivots>
struct type_rule_operand_types;

template <typename... Slots, typename... Pivots>
struct type_rule_operand_types<type_list<Slots...>, type_list<Pivots...>>
{
	using type = type_list<
		typename Slots::transform_type::template apply_type<
			typename type_list_element<
				Slots::pivot_index,
				type_list<Pivots...>
			>::type
		>::type...
	>;
};

/**
 * @brief Convenience alias for `type_rule_operand_types<Slots, Pivots>::type`.
 *
 * @tparam Slots type_list of type_slot, one per operand.
 * @tparam Pivots type_list of the resolved pivot element types.
 */
template <typename Slots, typename Pivots>
using type_rule_operand_types_t =
	typename type_rule_operand_types<Slots, Pivots>::type;

} // namespace rexlib
