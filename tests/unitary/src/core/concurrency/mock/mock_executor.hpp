// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/concurrency/executor.hpp>

#include <rexlib/core/concurrency/completion_notifier.hpp>
#include <rexlib/core/concurrency/task.hpp>

#include <memory>
#include <trompeloeil.hpp>

namespace rexlib
{

class mock_executor final
	: public executor
{
public:
	mock_executor() = default;

	MAKE_MOCK2(
		submit,
		void(
			std::unique_ptr<task> t,
			std::shared_ptr<completion_notifier> notifier
		),
		override
	);
};

} // namespace rexlib
