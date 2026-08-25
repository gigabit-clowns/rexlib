// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "operation.hpp"
#include "operation_id.hpp"

#include "../platform/attributes.hpp"
#include "../platform/dynamic_shared_object.h"

#include <typeinfo>

namespace rexlib
{

/**
 * @brief Report an operation that is not of the expected type.
 *
 * Defined out of line so that a downcast does not emit a throw site per
 * instantiation, of which there is one per operation and backend.
 *
 * @param got The operation that was passed.
 * @param expected Name of the operation that was expected.
 * @throws std::invalid_argument Always.
 */
REXLIB_NORETURN REXLIB_API
void throw_unexpected_operation(
	const operation &got,
	const char *expected
);

namespace detail
{

template <typename Op, typename = void>
struct operation_type_name
{
	static const char* get() noexcept
	{
		return typeid(Op).name();
	}
};

template <typename Op>
struct operation_type_name<
	Op,
	decltype(void(Op::get_static_descriptor()))
>
{
	static const char* get() noexcept
	{
		return Op::get_static_descriptor().get_name();
	}
};

} // namespace detail

/**
 * @brief Downcast an operation after checking its identity.
 *
 * Program builders reach an operation's parameters by downcasting it. Doing
 * so through the operation id rather than through RTTI matches how the
 * builder was selected in the first place, and accepts a subclass that
 * reports its base's identity, which is exactly the substitutability the
 * identity is supposed to express.
 *
 * @tparam Op The concrete operation type.
 * @param op The operation to be downcast.
 * @return const Op& The operation.
 * @throws std::invalid_argument When @p op is not an @p Op.
 */
template <typename Op>
const Op& operation_cast(const operation &op)
{
	if (op.get_id() != operation_id::of<Op>())
	{
		throw_unexpected_operation(
			op,
			detail::operation_type_name<Op>::get()
		);
	}

	return static_cast<const Op&>(op);
}

} // namespace rexlib
