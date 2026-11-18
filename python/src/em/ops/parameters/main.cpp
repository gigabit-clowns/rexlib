// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "ctf_optics.hpp"
#include "gridding_profile.hpp"
#include "gridding_window.hpp"

namespace rexlib
{
namespace em
{

namespace nb = nanobind;

void bind_parameters(nb::module_ &m)
{
	auto gridding_profile = declare_gridding_profile(m);
	auto gridding_window = declare_gridding_window(m);
	auto ctf_optics = declare_ctf_optics(m);

	define_gridding_profile(gridding_profile);
	define_gridding_window(gridding_window);
	define_ctf_optics(ctf_optics);
}

} // namespace em
} // namespace rexlib
