// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <backends/cpu/hardware/command_queue.hpp>

#include <rex/core/hardware/command.hpp>
#include <rex/core/span.hpp>

#include "../serial_pool.hpp"
#include "mock/mock_program.hpp"
#include "../../../core/hardware/mock/mock_buffer.hpp"
#include "../../../core/hardware/mock/mock_program.hpp"
#include "../../../core/hardware/mock/mock_event.hpp"
#include "../../../core/hardware/mock/mock_command_queue.hpp"

#include <memory>
#include <typeinfo>
#include <vector>

using namespace rex;
using namespace rex::cpu;

TEST_CASE(
	"cpu::command_queue should reject a null thread pool",
	"[cpu::command_queue]"
)
{
	CHECK_THROWS_AS(
		cpu::command_queue(nullptr),
		std::invalid_argument
	);
}

TEST_CASE(
	"cpu::command_queue should run its programs over the pool it was given",
	"[cpu::command_queue]"
)
{
	const auto pool = get_serial_pool();
	const cpu::command_queue queue(pool);
	CHECK( &queue.get_thread_pool() == pool.get() );
}

TEST_CASE(
	"cpu::command_queue::submit throws when the command has no program",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	const command cmd;
	CHECK_THROWS_AS( queue.submit(cmd), std::invalid_argument );
}

TEST_CASE(
	"cpu::command_queue::submit throws when the program is not a cpu::program",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	const command cmd(std::make_shared<xmipp4::mock_program>());
	CHECK_THROWS_AS( queue.submit(cmd), std::bad_cast );
}

TEST_CASE(
	"cpu::command_queue::submit calls execute with the bound operands",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());

	const auto prog = std::make_shared<cpu::mock_program>();
	const std::vector<std::shared_ptr<buffer>> outputs = { 
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>() 
	};
	const std::vector<std::shared_ptr<const buffer>> inputs = { 
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>() 
	};
	const std::vector<std::shared_ptr<buffer>> scratch = { 
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>() 
	};

	command cmd(prog);
	cmd.bind_outputs(make_span(outputs))
	   .bind_inputs(make_span(inputs))
	   .bind_scratch(make_span(scratch));

	REQUIRE_CALL(
		*prog,
		execute(
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_
		)
	)
		.LR_WITH(_1.data() == outputs.data() && _1.size() == outputs.size())
		.LR_WITH(_2.data() == inputs.data() && _2.size() == inputs.size())
		.LR_WITH(_3.data() == scratch.data() && _3.size() == scratch.size());

	REQUIRE_NOTHROW( queue.submit(cmd) );
}

TEST_CASE(
	"cpu::command_queue::signal should return without touching the event",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	mock_event event;
	REQUIRE_NOTHROW( queue.signal(event) );
}

TEST_CASE(
	"cpu::command_queue::wait should return without touching the event",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	mock_event event;
	REQUIRE_NOTHROW( queue.wait(event) );
}

TEST_CASE(
	"cpu::command_queue::try_cast returns the same object for a command_queue",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	xmipp4::command_queue &base = queue;
	CHECK( cpu::command_queue::try_cast(base) == &queue );
}

TEST_CASE(
	"cpu::command_queue::try_cast returns nullptr for a foreign command_queue",
	"[cpu::command_queue]"
)
{
	mock_command_queue queue;
	xmipp4::command_queue &base = queue;
	CHECK( cpu::command_queue::try_cast(base) == nullptr );
}

TEST_CASE(
	"cpu::command_queue::try_cast const overload preserves constness",
	"[cpu::command_queue]"
)
{
	const cpu::command_queue queue(get_serial_pool());
	const xmipp4::command_queue &base = queue;
	const cpu::command_queue *result = cpu::command_queue::try_cast(base);
	CHECK( result == &queue );
}
