// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/parameters/gridding_profile.hpp>

namespace rexlib
{
namespace em
{

const char* to_string(gridding_profile profile) noexcept
{
	switch (profile)
	{
	case gridding_profile::box: return "box";
	case gridding_profile::triangle: return "triangle";
	case gridding_profile::kaiser_bessel: return "kaiser_bessel";
	default: return "";
	}
}

std::ostream& operator<<(std::ostream &os, gridding_profile profile)
{
	return os << to_string(profile);
}

} // namespace em
} // namespace rexlib
