// SPDX-License-Identifier: GPL-3.0-only

#include <em/ops/parameters/check_pixel_size.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace em
{

double check_pixel_size(const char *operation, double pixel_size)
{
	if (!(std::isfinite(pixel_size) && pixel_size > 0.0))
	{
		std::ostringstream oss;
		oss << operation << ": the pixel size must be finite and positive, "
			<< "but it is " << pixel_size << ".";
		throw std::invalid_argument(oss.str());
	}
	return pixel_size;
}

} // namespace em
} // namespace rexlib
