// SPDX-License-Identifier: GPL-3.0-only

#include "plugin_loader.hpp"

#include <rexlib/core/exceptions/plugin_load_error.hpp>

namespace rexlib
{

static const plugin* query_plugin(const dynamic_library& lib)
{
	using get_plugin_function_type = const plugin* (*)();
	const std::string symbol_name = "rex_get_plugin";

	const auto func = reinterpret_cast<get_plugin_function_type>(
		lib.get_symbol(symbol_name.c_str())
	);
	
	if (!func)
	{
		throw plugin_load_error(
			symbol_name +
			" symbol could not be found in shared object."
		);
	}

	const auto* result = func();
	if(!result)
	{
		throw plugin_load_error(symbol_name + " returned NULL.");
	}

	return result;
}

plugin_loader::plugin_loader()
	: m_plugin(nullptr)
{
}

plugin_loader::plugin_loader(const std::string& path)
	: m_dynamic_library(path)
	, m_plugin(query_plugin(m_dynamic_library))
{
}

const plugin* plugin_loader::get_plugin() const noexcept
{
	return m_plugin;
}

bool plugin_loader::is_open() const noexcept
{
	return m_dynamic_library.is_open();
}

void plugin_loader::reset()
{
	*this = plugin_loader();
}

void plugin_loader::load(const std::string& path)
{
	*this = plugin_loader(path);
}

} // namespace rexlib
