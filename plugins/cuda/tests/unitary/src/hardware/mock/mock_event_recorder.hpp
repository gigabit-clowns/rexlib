// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/event_recorder.hpp>

#include <trompeloeil.hpp>

namespace rexlib
{
namespace cuda
{

class mock_event_recorder final
	: public event_recorder
{
public:
	MAKE_MOCK1(record, ticket(cudaStream_t), override);
	MAKE_MOCK1(release, void(ticket), noexcept override);
	MAKE_MOCK1(is_complete, bool(ticket), override);
	MAKE_MOCK1(wait, void(ticket), override);
	MAKE_MOCK2(enqueue_wait, void(cudaStream_t, ticket), override);
};

} // namespace cuda
} // namespace rexlib
