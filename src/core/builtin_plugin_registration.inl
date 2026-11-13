// SPDX-License-Identifier: GPL-3.0-only

#include "builtin_plugin_registration.hpp"

#include <memory>

namespace rexlib
{

template <typename Plugin, typename Registry>
inline
builtin_plugin_registration<Plugin, Registry>::builtin_plugin_registration(
	Registry &registry
)
{
	registry.add(std::make_unique<Plugin>());
}

} // namespace rexlib
