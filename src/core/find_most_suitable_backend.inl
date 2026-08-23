// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "find_most_suitable_backend.hpp"

#include <utility>

namespace rex
{

template <typename ForwardIte, typename F>
inline
ForwardIte find_most_suitable_backend(
	ForwardIte first, 
	ForwardIte last, 
	const F& suitability_evaluator
)
{
	std::pair<ForwardIte, backend_priority> best(
		last, 
		backend_priority::unsupported
	);

	for (auto ite = first; ite != last; ++ite)
	{
		const auto suitability = suitability_evaluator(*ite);
		if (suitability > best.second)
		{
			best = { ite, suitability };
		}
	}

	return best.first;
}

} // namespace rex
