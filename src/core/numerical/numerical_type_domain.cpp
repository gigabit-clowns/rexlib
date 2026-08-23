// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/numerical/numerical_type_domain.hpp>

namespace rex
{

std::ostream& operator<<(
	std::ostream &os,
	const numerical_type_domain &domain
)
{
	if (domain.empty())
	{
		return os << "<none>";
	}

	bool first = true;
	const auto count = static_cast<int>(numerical_type::count);
	for (int i = 0; i < count; ++i)
	{
		const auto type = static_cast<numerical_type>(i);
		if (domain.contains(type))
		{
			if (!first)
			{
				os << ", ";
			}
			os << type;
			first = false;
		}
	}

	return os;
}

} // namespace rex
