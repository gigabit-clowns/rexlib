// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/dispatch/operation_cast.hpp>

#include <sstream>
#include <stdexcept>

namespace rexlib
{

void throw_unexpected_operation(
	const operation &got,
	const char *expected
)
{
	std::ostringstream oss;
	oss << "Expected operation '" << expected << "', but got '"
		<< got.get_name() << "'.";
	throw std::invalid_argument(oss.str());
}

} // namespace rexlib
