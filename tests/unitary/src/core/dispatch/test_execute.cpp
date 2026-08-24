// SPDX-License-Identifier: GPL-3.0-only

#include <catch2/catch_test_macros.hpp>

#include <rex/core/dispatch/execute.hpp>
#include <rex/core/dispatch/execution_context.hpp>
#include <rex/core/ndarray/array.hpp>
#include <rex/core/ndarray/const_array_ref.hpp>
#include <rex/core/hardware/device_context.hpp>

#include "mock/mock_dispatcher.hpp"
#include "mock/mock_operation.hpp"

#include <memory>
#include <stdexcept>

using namespace rexlib;

namespace
{

class operation_execute_fixture
{
public:
    operation_execute_fixture()
        : dispatcher(std::make_shared<mock_dispatcher>())
        , context(device_context(), dispatcher)
    {
    }

protected:
    mock_operation op;
    std::shared_ptr<mock_dispatcher> dispatcher;
    execution_context context;
};

} // namespace


TEST_CASE(
    "execute throws std::invalid_argument when the dispatcher is null",
    "[operation_execute]"
)
{
    const execution_context ctx;
    mock_operation op;
    array output;
    const const_array_ref input;

    CHECK_THROWS_AS(
        execute(op, make_span(&output, 1), make_span(&input, 1), ctx),
        std::invalid_argument
    );
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute forwards the output and input operand spans to the dispatcher",
    "[operation_execute]"
)
{
    array output;
    const const_array_ref input;

    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 1);

    execute(op, make_span(&output, 1), make_span(&input, 1), context);
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute with null out allocates a fresh output array and dispatches",
    "[operation_execute]"
)
{
    const const_array_ref input;

    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 1);

    execute(op, make_span(&input, 1), context, nullptr);
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute with non-null out passes that array to the dispatcher",
    "[operation_execute]"
)
{
    const const_array_ref input;
    array out;

    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .LR_WITH(_2.data() == &out)
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 1);

    execute(op, make_span(&input, 1), context, &out);
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute_unary calls dispatch with a single-element input span",
    "[operation_execute]"
)
{
    const_array input;

    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 1);

    execute_unary(op, input, context);
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute_binary calls dispatch with a two-element input span",
    "[operation_execute]"
)
{
    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 2);

    execute_binary(op, const_array_ref(), const_array_ref(), context);
}

TEST_CASE_METHOD(
    operation_execute_fixture,
    "execute_ternary calls dispatch with a three-element input span",
    "[operation_execute]"
)
{
    REQUIRE_CALL(
        *dispatcher,
        dispatch(trompeloeil::_, trompeloeil::_, trompeloeil::_, trompeloeil::_)
    )
        .WITH(_2.size() == 1)
        .WITH(_3.size() == 3);

    execute_ternary(op, const_array_ref(), const_array_ref(), const_array_ref(), context);
}
