// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/em/image/image_sink.hpp>

#include <rexlib/core/concurrency/completion.hpp>
#include <rexlib/core/ndarray/const_array.hpp>
#include <rexlib/em/image/image_transaction_plan.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace em
{

class mock_image_sink final
	: public image_sink
{
public:
	mock_image_sink() = default;

	MAKE_CONST_MOCK2(
		write,
		std::shared_ptr<completion>(
			const_array source,
			const image_transaction_plan &plan
		),
		override
	);

	MAKE_MOCK0(flush, void(), override);
};

} // namespace em
} // namespace rexlib
