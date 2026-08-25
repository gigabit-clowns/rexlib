// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/layout/index.hpp>

#include <sstream>
#include <stdexcept>

namespace rexlib 
{

std::size_t sanitize_index(std::ptrdiff_t index, std::size_t extent)
{
	auto result = index;
	
	if (result < 0)
	{
		if (result < -static_cast<std::ptrdiff_t>(extent))
		{
			std::ostringstream oss;
			oss << "Reverse index " << result 
				<< " is out of bounds for extent " << extent;
			throw std::out_of_range(oss.str());
		}

		result += extent;
	}
	else if (result >= static_cast<std::ptrdiff_t>(extent))
	{
		std::ostringstream oss;
		oss << "Index " << result << " is out of bounds for extent " << extent;
		throw std::out_of_range(oss.str());
	}

	return static_cast<std::size_t>(result);
}

} // namespace rexlib
