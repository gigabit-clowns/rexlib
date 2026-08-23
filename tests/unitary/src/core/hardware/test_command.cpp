// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/hardware/command.hpp>
#include <rex/core/span.hpp>

#include "mock/mock_buffer.hpp"
#include "mock/mock_program.hpp"

#include <memory>
#include <vector>

using namespace rex;

TEST_CASE(
    "command default constructor produces null program and empty spans",
    "[command]"
)
{
    const command cmd;
    CHECK( cmd.get_program() == nullptr );
    CHECK( cmd.get_outputs().empty() );
    CHECK( cmd.get_inputs().empty() );
    CHECK( cmd.get_scratch().empty() );
}

TEST_CASE(
    "command program constructor sets the program and leaves spans empty",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const command cmd(prog);

    CHECK( cmd.get_program() == prog );
    CHECK( cmd.get_outputs().empty() );
    CHECK( cmd.get_inputs().empty() );
    CHECK( cmd.get_scratch().empty() );
}

TEST_CASE(
    "command::bind_outputs stores a non-owning view and returns *this",
    "[command]"
)
{
    const auto buf = std::make_shared<mock_buffer>();
    const std::vector<std::shared_ptr<buffer>> buffers = { buf };
    const auto sp = make_span(buffers);

    command cmd;
    command &returned = cmd.bind_outputs(sp);

    CHECK( &returned == &cmd );
    CHECK( cmd.get_outputs().data() == sp.data() );
    CHECK( cmd.get_outputs().size() == 1 );
}

TEST_CASE(
    "command::bind_inputs stores a non-owning view and returns *this",
    "[command]"
)
{
    const auto buf = std::make_shared<mock_buffer>();
    const std::vector<std::shared_ptr<const buffer>> buffers = { buf };
    const auto sp = make_span(buffers);

    command cmd;
    command &returned = cmd.bind_inputs(sp);

    CHECK( &returned == &cmd );
    CHECK( cmd.get_inputs().data() == sp.data() );
    CHECK( cmd.get_inputs().size() == 1 );
}

TEST_CASE(
    "command::bind_scratch stores a non-owning view and returns *this",
    "[command]"
)
{
    const auto buf = std::make_shared<mock_buffer>();
    const std::vector<std::shared_ptr<buffer>> buffers = { buf };
    const auto sp = make_span(buffers);

    command cmd;
    command &returned = cmd.bind_scratch(sp);

    CHECK( &returned == &cmd );
    CHECK( cmd.get_scratch().data() == sp.data() );
    CHECK( cmd.get_scratch().size() == 1 );
}

TEST_CASE(
    "command bind methods can be chained",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const std::vector<std::shared_ptr<buffer>> outputs = { std::make_shared<mock_buffer>() };
    const std::vector<std::shared_ptr<const buffer>> inputs = { std::make_shared<mock_buffer>() };
    const std::vector<std::shared_ptr<buffer>> scratch = { std::make_shared<mock_buffer>() };

    command cmd(prog);
    cmd.bind_outputs(make_span(outputs))
       .bind_inputs(make_span(inputs))
       .bind_scratch(make_span(scratch));

    CHECK( cmd.get_outputs().size() == 1 );
    CHECK( cmd.get_inputs().size() == 1 );
    CHECK( cmd.get_scratch().size() == 1 );
}

TEST_CASE(
    "command copy constructor copies program and span views",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const std::vector<std::shared_ptr<buffer>> buffers = { std::make_shared<mock_buffer>() };

    command original(prog);
    original.bind_outputs(make_span(buffers));

    const command copy(original);

    CHECK( copy.get_program() == prog );
    CHECK( copy.get_outputs().data() == buffers.data() );
    CHECK( copy.get_outputs().size() == 1 );
}

TEST_CASE(
    "command move constructor transfers program and span views",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const std::vector<std::shared_ptr<buffer>> buffers = { std::make_shared<mock_buffer>() };

    command original(prog);
    original.bind_outputs(make_span(buffers));

    const command moved(std::move(original));

    CHECK( moved.get_program() == prog );
    CHECK( moved.get_outputs().data() == buffers.data() );
    CHECK( moved.get_outputs().size() == 1 );
}

TEST_CASE(
    "command copy assignment copies program and span views",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const std::vector<std::shared_ptr<buffer>> buffers = { std::make_shared<mock_buffer>() };

    command original(prog);
    original.bind_outputs(make_span(buffers));

    command copy;
    copy = original;

    CHECK( copy.get_program() == prog );
    CHECK( copy.get_outputs().data() == buffers.data() );
    CHECK( copy.get_outputs().size() == 1 );
}

TEST_CASE(
    "command move assignment transfers program and span views",
    "[command]"
)
{
    const auto prog = std::make_shared<mock_program>();
    const std::vector<std::shared_ptr<buffer>> buffers = { std::make_shared<mock_buffer>() };

    command original(prog);
    original.bind_outputs(make_span(buffers));

    command moved;
    moved = std::move(original);

    CHECK( moved.get_program() == prog );
    CHECK( moved.get_outputs().data() == buffers.data() );
    CHECK( moved.get_outputs().size() == 1 );
}
