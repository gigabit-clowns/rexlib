// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/dispatch/operation.hpp>

#include <sstream>

namespace rex
{

operation::operation() noexcept = default;
operation::~operation() = default;

operation_id operation::get_id() const noexcept
{
	return operation_id(typeid(*this));
}

} // namespace rex
