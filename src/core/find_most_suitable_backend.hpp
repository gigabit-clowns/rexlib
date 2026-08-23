// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <xmipp4/core/backend_priority.hpp>

namespace rex
{

/**
 * @brief Find the item with the highest backend_priority in the range.
 * 
 * @tparam ForwardIte Forward iterator.
 * @tparam F Function of signature (ForwardIte::value_type) -> backend_priority.
 * @param first First element in the range.
 * @param last Past the en element in the range.
 * @param suitability_evaluator Function to evaluate the backend_priority of
 * each item.
 * @return ForwardIte Most suitable backend. last if none of them is supported.
 */
template <typename ForwardIte, typename F>
ForwardIte find_most_suitable_backend(
	ForwardIte first, 
	ForwardIte last, 
	const F& suitability_evaluator
);

} // namespace rex

#include "find_most_suitable_backend.inl"
