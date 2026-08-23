// SPDX-License-Identifier: GPL-3.0-only

#include "parallel_grain.hpp"

#include <backends/cpu/config.hpp>

#include <core/logger.hpp>

#include <algorithm>
#include <cstdlib>

namespace rex
{
namespace cpu
{

static const char REX_GRAIN_SIZE_ENV_VARIABLE[] =
	"REX_PARALLEL_GRAIN_SIZE";

namespace
{

std::size_t read_parallel_grain_size()
{
	std::size_t result = REX_PARALLEL_GRAIN_SIZE;

	const char* environment_variable;
	if ((environment_variable = std::getenv(REX_GRAIN_SIZE_ENV_VARIABLE)))
	{
		char *end = nullptr;
		const auto parsed = std::strtoul(environment_variable, &end, 10);
		if (end != environment_variable && *end == '\0' && parsed > 0)
		{
			result = static_cast<std::size_t>(parsed);
		}
		else
		{
			REX_LOG_WARN(
				"Ignoring {}=\"{}\": expected a positive integer.",
				REX_GRAIN_SIZE_ENV_VARIABLE,
				environment_variable
			);
		}
	}

	return result;
}

} // anonymous namespace

std::size_t get_parallel_grain_size() noexcept
{
	// Read once rather than once per loop. The environment does not change
	// under a running process in any way this should follow, and every loop
	// of the backend asks for this every time it runs.
	static const std::size_t result = read_parallel_grain_size();
	return result;
}

std::size_t grain_for_cost(std::size_t cost_per_iteration) noexcept
{
	const auto budget = get_parallel_grain_size();
	if (cost_per_iteration < 2)
	{
		return budget;
	}

	// An iteration costing more than the whole budget is still worth handing
	// over one at a time, which is what a batch of matrix products wants.
	return std::max<std::size_t>(budget / cost_per_iteration, 1);
}

} // namespace cpu
} // namespace rex
