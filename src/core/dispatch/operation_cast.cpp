// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/dispatch/operation_cast.hpp>

#include <sstream>
#include <stdexcept>

namespace rex
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

} // namespace rex
