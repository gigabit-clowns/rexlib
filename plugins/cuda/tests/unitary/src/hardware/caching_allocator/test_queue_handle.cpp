// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <hardware/caching_allocator/queue_handle.hpp>

#include "test_queue.hpp"

#include <cstdint>
#include <set>

using namespace xmipp4;

TEST_CASE(
	"a default constructed queue_handle should refer to no queue",
	"[queue_handle]"
)
{
	const cuda::queue_handle handle;

	CHECK( !handle );
	CHECK( handle.get_stream() == nullptr );
	CHECK( handle.get_ordinal() < 0 );
}

TEST_CASE(
	"a queue_handle built from its components should report them back",
	"[queue_handle]"
)
{
	auto *const stream = cuda::make_test_queue(0).get_stream();
	const cuda::queue_handle handle(stream, 3);

	CHECK( static_cast<bool>(handle) );
	CHECK( handle.get_stream() == stream );
	CHECK( handle.get_ordinal() == 3 );
}

TEST_CASE(
	"queue_handle equality should only consider the stream",
	"[queue_handle]"
)
{
	auto *const stream = cuda::make_test_queue(0).get_stream();
	const cuda::queue_handle handle(stream, 0);
	const cuda::queue_handle same_stream(stream, 1);
	const auto other_stream = cuda::make_test_queue(1);

	// The ordinal is derived from the stream, so it cannot disagree in
	// practice, and letting it into the comparison would only make two names
	// for one queue look like two queues.
	CHECK( handle == same_stream );
	CHECK( handle != other_stream );
}

TEST_CASE(
	"queue_handle should order strictly and consistently",
	"[queue_handle]"
)
{
	const cuda::queue_handle none;
	const auto first = cuda::make_test_queue(0);
	const auto second = cuda::make_test_queue(1);

	CHECK( none < first );
	CHECK( first < second );
	CHECK( !(second < first) );
	CHECK( !(first < first) );

	// The point of the order is to key a container, so it has to actually
	// work as one.
	const std::set<cuda::queue_handle> handles = {second, first, none, first};
	REQUIRE( handles.size() == 3 );
	CHECK( *handles.begin() == none );
	CHECK( *handles.rbegin() == second );
}
