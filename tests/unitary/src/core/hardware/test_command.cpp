// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rexlib/core/hardware/command.hpp>
#include <rexlib/core/span.hpp>

#include "mock/mock_buffer.hpp"
#include "mock/mock_command_timeline.hpp"
#include "mock/mock_program.hpp"

#include <memory>
#include <utility>
#include <vector>

using namespace rexlib;

TEST_CASE(
	"command default constructor produces null program and empty bindings",
	"[command]"
)
{
	const command cmd;
	CHECK( cmd.get_program() == nullptr );
	CHECK( cmd.get_outputs().empty() );
	CHECK( cmd.get_inputs().empty() );
	CHECK( cmd.get_scratch().empty() );
	CHECK( cmd.get_dependencies().empty() );
}

TEST_CASE(
	"command program constructor sets the program and leaves bindings empty",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();
	const command cmd(prog);

	CHECK( cmd.get_program() == prog );
	CHECK( cmd.get_outputs().empty() );
	CHECK( cmd.get_inputs().empty() );
	CHECK( cmd.get_scratch().empty() );
	CHECK( cmd.get_dependencies().empty() );
}

TEST_CASE(
	"command::bind_outputs holds the buffers and returns *this",
	"[command]"
)
{
	const auto buf = std::make_shared<mock_buffer>();

	command cmd;
	command &returned = cmd.bind_outputs({ buf });

	CHECK( &returned == &cmd );
	REQUIRE( cmd.get_outputs().size() == 1 );
	CHECK( cmd.get_outputs()[0] == buf );
}

TEST_CASE(
	"command::bind_inputs holds the buffers and returns *this",
	"[command]"
)
{
	const auto buf = std::make_shared<mock_buffer>();

	command cmd;
	command &returned = cmd.bind_inputs({ buf });

	CHECK( &returned == &cmd );
	REQUIRE( cmd.get_inputs().size() == 1 );
	CHECK( cmd.get_inputs()[0] == buf );
}

TEST_CASE(
	"command::bind_scratch holds the buffers and returns *this",
	"[command]"
)
{
	const auto buf = std::make_shared<mock_buffer>();

	command cmd;
	command &returned = cmd.bind_scratch({ buf });

	CHECK( &returned == &cmd );
	REQUIRE( cmd.get_scratch().size() == 1 );
	CHECK( cmd.get_scratch()[0] == buf );
}

TEST_CASE(
	"command::bind_dependencies holds the tokens and returns *this",
	"[command]"
)
{
	const auto timeline = std::make_shared<mock_command_timeline>();

	command cmd;
	command &returned = cmd.bind_dependencies({
		command_token(timeline, 1),
		command_token(timeline, 2)
	});

	CHECK( &returned == &cmd );
	REQUIRE( cmd.get_dependencies().size() == 2 );
	CHECK( cmd.get_dependencies()[0].get_timeline() == timeline );
	CHECK( cmd.get_dependencies()[0].get_id() == 1u );
	CHECK( cmd.get_dependencies()[1].get_timeline() == timeline );
	CHECK( cmd.get_dependencies()[1].get_id() == 2u );
}

TEST_CASE(
	"command bind methods can be chained",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();

	command cmd(prog);
	cmd.bind_outputs({ std::make_shared<mock_buffer>() })
		.bind_inputs({ std::make_shared<mock_buffer>() })
		.bind_scratch({ std::make_shared<mock_buffer>() })
		.bind_dependencies({ command_token() });

	CHECK( cmd.get_outputs().size() == 1 );
	CHECK( cmd.get_inputs().size() == 1 );
	CHECK( cmd.get_scratch().size() == 1 );
	CHECK( cmd.get_dependencies().size() == 1 );
}

TEST_CASE(
	"command keeps what is bound after the caller's vector is gone",
	"[command]"
)
{
	const auto buf = std::make_shared<mock_buffer>();

	command cmd;
	{
		const std::vector<std::shared_ptr<buffer>> buffers = { buf };
		cmd.bind_outputs(buffers);
	}

	REQUIRE( cmd.get_outputs().size() == 1 );
	CHECK( cmd.get_outputs()[0] == buf );
}

TEST_CASE(
	"command takes over a vector that is moved into it",
	"[command]"
)
{
	std::vector<std::shared_ptr<buffer>> buffers = {
		std::make_shared<mock_buffer>(),
		std::make_shared<mock_buffer>()
	};
	const auto *data = buffers.data();

	command cmd;
	cmd.bind_outputs(std::move(buffers));

	CHECK( cmd.get_outputs().data() == data );
	CHECK( cmd.get_outputs().size() == 2 );
}

TEST_CASE(
	"command binding again replaces what was bound",
	"[command]"
)
{
	const auto first = std::make_shared<mock_buffer>();
	const auto second = std::make_shared<mock_buffer>();

	command cmd;
	cmd.bind_outputs({ first, first });
	cmd.bind_outputs({ second });

	REQUIRE( cmd.get_outputs().size() == 1 );
	CHECK( cmd.get_outputs()[0] == second );
}

TEST_CASE(
	"command copy constructor copies the program and what is bound",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();
	const auto buf = std::make_shared<mock_buffer>();

	command original(prog);
	original.bind_outputs({ buf });

	const command copy(original);

	CHECK( copy.get_program() == prog );
	REQUIRE( copy.get_outputs().size() == 1 );
	CHECK( copy.get_outputs()[0] == buf );
	CHECK( copy.get_outputs().data() != original.get_outputs().data() );
}

TEST_CASE(
	"command move constructor transfers the program and what is bound",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();
	const auto buf = std::make_shared<mock_buffer>();

	command original(prog);
	original.bind_outputs({ buf });

	const command moved(std::move(original));

	CHECK( moved.get_program() == prog );
	REQUIRE( moved.get_outputs().size() == 1 );
	CHECK( moved.get_outputs()[0] == buf );
}

TEST_CASE(
	"command copy assignment copies the program and what is bound",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();
	const auto buf = std::make_shared<mock_buffer>();

	command original(prog);
	original.bind_outputs({ buf });

	command copy;
	copy = original;

	CHECK( copy.get_program() == prog );
	REQUIRE( copy.get_outputs().size() == 1 );
	CHECK( copy.get_outputs()[0] == buf );
	CHECK( copy.get_outputs().data() != original.get_outputs().data() );
}

TEST_CASE(
	"command move assignment transfers the program and what is bound",
	"[command]"
)
{
	const auto prog = std::make_shared<mock_program>();
	const auto buf = std::make_shared<mock_buffer>();

	command original(prog);
	original.bind_outputs({ buf });

	command moved;
	moved = std::move(original);

	CHECK( moved.get_program() == prog );
	REQUIRE( moved.get_outputs().size() == 1 );
	CHECK( moved.get_outputs()[0] == buf );
}
