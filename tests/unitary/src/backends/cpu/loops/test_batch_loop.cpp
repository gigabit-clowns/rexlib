// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <backends/cpu/loops/batch_loop.hpp>
#include <backends/cpu/loops/indices/element_index_tags.hpp>
#include <backends/cpu/loops/layouts/index_operands.hpp>
#include <backends/cpu/loops/loop_schedule.hpp>

#include <rexlib/backends/cpu/thread_pool.hpp>
#include <rexlib/core/layout/joint_layout_builder.hpp>
#include <rexlib/core/span.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

/**
 * @brief A batch of a given count over one operand of stride 10.
 */
template <typename Indexing = no_index_tag>
joint_layout make_batch_layout(
	std::size_t count,
	Indexing indexing = Indexing()
)
{
	const std::array<std::size_t, 1> extents = {{ count }};
	const std::array<std::ptrdiff_t, 1> strides = {{ 10 }};
	joint_layout_builder builder;
	builder.set_extents(make_span(extents));
	builder.add_operand(make_span(extents), make_span(strides), 0);
	add_index_operands(builder, make_span(extents), indexing);
	return builder.build();
}

} // anonymous namespace

TEST_CASE(
	"run_batch_loop should visit every batch element once, placed",
	"[batch_loop]"
)
{
	thread_pool pool(3);
	std::vector<int> values(80, 0);

	run_batch_loop(
		[] (const loop_schedule&, int *value)
		{
			++*value;
		},
		make_batch_layout(8),
		no_index_tag(),
		loop_schedule(pool, 1),
		1000000,
		0,
		values.data()
	);

	for (std::size_t i = 0; i < values.size(); ++i)
	{
		CHECK( values[i] == (i % 10 == 0 ? 1 : 0) );
	}
}

TEST_CASE(
	"run_batch_loop should hand the visitor the threads when the batch "
	"elements are too few",
	"[batch_loop]"
)
{
	thread_pool pool(3);
	REQUIRE( pool.get_size() > 1 );
	std::vector<int> values(10, 0);
	std::atomic<std::size_t> inner_concurrency(0);

	run_batch_loop(
		[&inner_concurrency] (const loop_schedule &inner, int*)
		{
			inner_concurrency = inner.get_concurrency();
		},
		make_batch_layout(1),
		no_index_tag(),
		loop_schedule(pool, 1),
		1000000,
		1000000,
		values.data()
	);

	CHECK( inner_concurrency > 1 );
}

TEST_CASE(
	"run_batch_loop should keep the work of a batch element on one thread "
	"when it cannot be spread",
	"[batch_loop]"
)
{
	thread_pool pool(3);
	std::vector<int> values(10, 0);
	std::atomic<std::size_t> inner_concurrency(0);

	run_batch_loop(
		[&inner_concurrency] (const loop_schedule &inner, int*)
		{
			inner_concurrency = inner.get_concurrency();
		},
		make_batch_layout(1),
		no_index_tag(),
		loop_schedule(pool, 1),
		1000000,
		0,
		values.data()
	);

	CHECK( inner_concurrency == 1 );
}

TEST_CASE(
	"run_batch_loop should hand the visitor the index it is asked for",
	"[batch_loop]"
)
{
	std::vector<std::size_t> seen;

	run_batch_loop(
		[&seen] (const loop_schedule&, int*, std::size_t index)
		{
			seen.push_back(index);
		},
		make_batch_layout(3, linear_index_tag()),
		linear_index_tag(),
		loop_schedule(),
		1,
		0,
		static_cast<int*>(nullptr)
	);

	CHECK( seen == std::vector<std::size_t>{ 0, 1, 2 } );
}
