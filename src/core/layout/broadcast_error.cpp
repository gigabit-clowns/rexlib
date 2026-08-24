// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/layout/broadcast_error.hpp>

#include <sstream>

namespace rexlib 
{
		
static 
void format_extents(std::ostream &os, const std::vector<std::size_t>& extents) 
{
	const auto n = extents.size();

	os << "(";
	for (size_t i = 0; i < n; ++i) {
		os << extents[i];
		if (i + 1 < n) os << ", ";
	}
	os << ")";
}

static 
std::string build_message(
	const std::vector<std::size_t>& a,
	const std::vector<std::size_t>& b,
	const char *context
) 
{
	std::ostringstream oss;

	oss << "Broadcast error: extents ";
	format_extents(oss, a);
	oss << " and ";
	format_extents(oss, b);
	oss << " are not compatible";

	if (context) 
	{
		oss << " in " << context;
	}

	oss << ".";
	return oss.str();
}


broadcast_error::broadcast_error(
	std::vector<std::size_t> first_extents,
	std::vector<std::size_t> second_extents,
	const char *context
) 
	: runtime_error(build_message(first_extents, second_extents, context))
	, m_first_extents(std::move(first_extents))
	, m_second_extents(std::move(second_extents))
{
}

const std::vector<std::size_t>& 
broadcast_error::get_first_extents() const noexcept
{
	return m_first_extents;
}

const std::vector<std::size_t>& 
broadcast_error::get_second_extents() const noexcept
{
	return m_second_extents;
}

} // namespace rexlib
