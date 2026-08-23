// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/dynamic_shared_object.h"

#include <stdexcept>
#include <vector>

namespace rex 
{

class broadcast_error
	: public std::runtime_error
{
public:
	REXLIB_API broadcast_error(
		std::vector<std::size_t> first_extents,
		std::vector<std::size_t> second_extents,
		const char *context = nullptr
	);

	REXLIB_API
	const std::vector<std::size_t>& get_first_extents() const noexcept;
	REXLIB_API
	const std::vector<std::size_t>& get_second_extents() const noexcept;

private:
	std::vector<std::size_t> m_first_extents;
	std::vector<std::size_t> m_second_extents;
};

} // namespace rex
