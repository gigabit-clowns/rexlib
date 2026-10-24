// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/numerical/numerical_type.hpp>
#include <rexlib/core/numerical/numerical_type_dispatch.hpp>

#include "safe_numerical_type_of.hpp"

namespace rexlib
{
namespace test
{

/**
 * @brief Check that both halves of a transform agree over its domain.
 *
 * For every type in the transform's domain, the type `apply_type` yields
 * must be the type `apply` returns.
 *
 * @tparam Transform The type transform.
 */
template <typename Transform>
void check_type_transform_consistency()
{
	const auto &domain = Transform::domain_type::get();

	const auto count = static_cast<int>(numerical_type::count);
	for (int i = 0; i < count; ++i)
	{
		const auto type = static_cast<numerical_type>(i);
		if (!domain.contains(type))
		{
			continue;
		}

		const auto expected = Transform::apply(type);

		const auto got = dispatch_numerical_types(
			[] (auto tag)
			{
				using source_type = typename decltype(tag)::type;
				using result_type = typename Transform::template
					apply_type<source_type>::type;
				return safe_numerical_type_of<result_type>::value;
			},
			type
		);

		INFO( "applied to " << type );
		CHECK( got == expected );
	}
}

} // namespace test
} // namespace rexlib
