// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_source.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/array.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_source final
	: public image_source
{
public:
	mock_image_source() = default;

	MAKE_CONST_MOCK2(
		read,
		std::shared_ptr<completion>(
			array destination,
			const image_transaction_plan &plan
		),
		override
	);
};

} // namespace em
} // namespace rexlib
