// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>

namespace rexlib
{
namespace em
{

/**
 * @brief Appends a factory for @p Format to a registry upon construction.
 *
 * An instance at namespace scope registers its format during static
 * initialization.
 *
 * @tparam Format The concrete format type. Must be default constructible.
 * @tparam Registry The registry to append to, which decides whether the
 * format is registered for reading or for writing. Any type that names its
 * format interface as @c format_type and takes a factory of it through
 * @c add.
 */
template <typename Format, typename Registry>
class image_format_registration
{
public:
	/**
	 * @brief Append a factory for @p Format to a registry.
	 *
	 * @param registry The registry to append to.
	 */
	explicit image_format_registration(Registry &registry);

private:
	static std::unique_ptr<typename Registry::format_type> create_format();
};

} // namespace em
} // namespace rexlib

#include "image_format_registration.inl"
