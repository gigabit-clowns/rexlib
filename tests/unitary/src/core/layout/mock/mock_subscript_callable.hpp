// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/layout/subscript_tags.hpp>
#include <rexlib/core/layout/slice.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{

class mock_subscript_callable
{
public:
	MAKE_MOCK1(function_call, void(ellipsis_tag), const);
	MAKE_MOCK1(function_call, void(new_axis_tag), const);
	MAKE_MOCK1(function_call, void(std::ptrdiff_t), const);
	MAKE_MOCK1(function_call, void(slice), const);

	template <typename T>
	void operator()(T &&arg) const
	{
		function_call(std::forward<T>(arg));
	}
};

} // namespace rexlib
