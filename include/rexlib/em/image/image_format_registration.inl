// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "image_format_registration.hpp"

namespace rexlib
{
namespace em
{

template <typename Format, typename Registry>
inline
image_format_registration<Format, Registry>::image_format_registration(
	Registry &registry
)
{
	registry.add(&create_format);
}

template <typename Format, typename Registry>
inline
std::unique_ptr<typename Registry::format_type>
image_format_registration<Format, Registry>::create_format()
{
	return std::make_unique<Format>();
}

} // namespace em
} // namespace rexlib
