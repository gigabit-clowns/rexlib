// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <hardware/caching_allocator/event_recorder.hpp>

#include <hardware/caching_allocator/queue_handle.hpp>

#include <trompeloeil.hpp>

namespace xmipp4
{
namespace cuda
{

class mock_event_recorder final
	: public event_recorder
{
public:
	MAKE_MOCK1(record, ticket(const queue_handle&), override);
	MAKE_MOCK1(release, void(ticket), noexcept override);
	MAKE_MOCK1(is_complete, bool(ticket), override);
	MAKE_MOCK1(wait, void(ticket), override);
	MAKE_MOCK2(enqueue_wait, void(const queue_handle&, ticket), override);
};

} // namespace cuda
} // namespace xmipp4
