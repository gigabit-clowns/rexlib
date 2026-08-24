// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rex/core/hardware/program.hpp>
#include <rex/core/meta/type_list.hpp>
#include <rex/core/numerical/numerical_type.hpp>
#include <rex/core/numerical/numerical_type_traits.hpp>

#include <trompeloeil.hpp>

#include <memory>
#include <vector>

namespace rexlib
{

/**
 * @brief Program factory mock for the type dispatcher tests.
 *
 * operator() reduces the resolved element type_lists to their runtime
 * numerical_type so they can be matched with trompeloeil, forwarding to the
 * mocked call. It is non-const so it binds as the lvalue reference the
 * dispatchers forward.
 */
class mock_factory
{
public:
	MAKE_MOCK2(
		call,
		std::shared_ptr<program>(
			std::vector<numerical_type>,
			std::vector<numerical_type>
		)
	);

	template <typename... Outs, typename... Ins>
	std::shared_ptr<program> operator()(
		type_list<Outs...>,
		type_list<Ins...>
	)
	{
		return call(
			std::vector<numerical_type>{ numerical_type_of<Outs>::value... },
			std::vector<numerical_type>{ numerical_type_of<Ins>::value... }
		);
	}
};

} // namespace rexlib
