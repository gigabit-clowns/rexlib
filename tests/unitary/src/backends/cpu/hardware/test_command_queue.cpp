// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <backends/cpu/hardware/command_queue.hpp>

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/span.hpp>

#include "../serial_pool.hpp"
#include "mock/mock_program.hpp"
#include "../../../core/hardware/mock/mock_buffer.hpp"
#include "../../../core/hardware/mock/mock_program.hpp"
#include "../../../core/hardware/mock/mock_command_queue.hpp"
#include "../../../core/hardware/mock/mock_command_timeline.hpp"

#include <algorithm>
#include <memory>
#include <typeinfo>
#include <vector>

using namespace rexlib;
using namespace rexlib::cpu;

namespace
{

template <typename T>
bool holds(span<const T> actual, const std::vector<T> &expected)
{
	return std::equal(
		actual.begin(), actual.end(),
		expected.begin(), expected.end()
	);
}

} // anonymous namespace

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
	const command cmd(std::make_shared<rexlib::mock_program>());
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
	cmd.bind_outputs(outputs)
	   .bind_inputs(inputs)
	   .bind_scratch(scratch);

	REQUIRE_CALL(
		*prog,
		execute(
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_
		)
	)
		.LR_WITH(holds(_1, outputs))
		.LR_WITH(holds(_2, inputs))
		.LR_WITH(holds(_3, scratch));

	REQUIRE_NOTHROW( queue.submit(cmd) );
}

TEST_CASE(
	"cpu::command_queue::submit returns a token for no command",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());

	const auto prog = std::make_shared<cpu::mock_program>();
	const command cmd(prog);

	REQUIRE_CALL(
		*prog,
		execute(
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_
		)
	);

	const auto token = queue.submit(cmd);

	CHECK( token.get_timeline() == nullptr );
	CHECK( token.is_complete() );
}

TEST_CASE(
	"cpu::command_queue::submit waits for its dependencies before it runs "
	"the program",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());

	const auto prog = std::make_shared<cpu::mock_program>();
	const auto timeline = std::make_shared<mock_command_timeline>();
	const std::vector<command_token> dependencies = {
		command_token(timeline, 3),
		command_token(),
		command_token(timeline, 5)
	};

	command cmd(prog);
	cmd.bind_dependencies(dependencies);

	trompeloeil::sequence seq;
	REQUIRE_CALL(*timeline, wait(3u))
		.IN_SEQUENCE(seq);
	REQUIRE_CALL(*timeline, wait(5u))
		.IN_SEQUENCE(seq);
	REQUIRE_CALL(
		*prog,
		execute(
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_,
			trompeloeil::_
		)
	)
		.IN_SEQUENCE(seq);

	queue.submit(cmd);
}

TEST_CASE(
	"cpu::command_queue::try_cast returns the same object for a command_queue",
	"[cpu::command_queue]"
)
{
	cpu::command_queue queue(get_serial_pool());
	rexlib::command_queue &base = queue;
	CHECK( cpu::command_queue::try_cast(base) == &queue );
}

TEST_CASE(
	"cpu::command_queue::try_cast returns nullptr for a foreign command_queue",
	"[cpu::command_queue]"
)
{
	mock_command_queue queue;
	rexlib::command_queue &base = queue;
	CHECK( cpu::command_queue::try_cast(base) == nullptr );
}

TEST_CASE(
	"cpu::command_queue::try_cast const overload preserves constness",
	"[cpu::command_queue]"
)
{
	const cpu::command_queue queue(get_serial_pool());
	const rexlib::command_queue &base = queue;
	const cpu::command_queue *result = cpu::command_queue::try_cast(base);
	CHECK( result == &queue );
}
