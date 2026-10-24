// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/operation/operation_cast.hpp>

#include <sstream>
#include <stdexcept>

namespace rexlib
{

namespace
{

template <typename Expected>
REXLIB_NORETURN
void throw_mismatch(const operation &got, const Expected &expected)
{
	std::ostringstream oss;
	oss << "Expected operation '" << expected << "', but got '"
		<< got.get_descriptor() << "'.";
	throw std::invalid_argument(oss.str());
}

} // anonymous namespace

void throw_unexpected_operation(
	const operation &got,
	const operation_descriptor &expected
)
{
	throw_mismatch(got, expected);
}

void throw_unexpected_operation(
	const operation &got,
	const char *expected
)
{
	throw_mismatch(got, expected);
}

} // namespace rexlib
