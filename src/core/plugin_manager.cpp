// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/plugin_manager.hpp>

#include "plugin_loader.hpp"

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/exceptions/plugin_load_error.hpp>

#include <core/logger.hpp>
#include <rexlib/core/platform/operating_system.h>

#include <vector>
#include <algorithm>
#include <functional>
#include <system_error>
#include <cstdlib>

#include <boost/filesystem.hpp>

static const char REXLIB_PLUGINS_DIRECTORY_NAME[] = "rexlib-plugins";
static const char REXLIB_PLUGINS_ENV_VARIABLE[] = "REXLIB_PLUGINS_PATH";

namespace rexlib
{

static void try_load_plugin(
	plugin_manager &manager,
	const std::string &path
)
{
	try
	{
		manager.load_plugin(path);
	}
	catch(const plugin_load_error& error)
	{
		REXLIB_LOG_ERROR(
			"Failed to load plugin from {}: {}", path, error.what()
		);
	}
	catch(const std::system_error& error)
	{
		REXLIB_LOG_ERROR(
			"Failed to load shared library {}: {}", path, error.what()
		);
	}
}

class plugin_manager::implementation
{
public:
	implementation() = default;

	void add_plugin(const plugin& plugin)
	{
		m_plugins.emplace_back(plugin);
	}

	const plugin* load_plugin(const std::string &path)
	{
		const auto ite = m_loaders.emplace(m_loaders.cend(), path);
		const auto* plugin = ite->get_plugin();
		if (plugin)
		{
			add_plugin(*plugin);
		}
		else
		{
			m_loaders.erase(ite); // Did not load anything
		}

		return plugin;
	}

	std::size_t get_plugin_count() const noexcept
	{
		return m_plugins.size();
	}

	const plugin& get_plugin(std::size_t index) const
	{
		return m_plugins.at(index);
	}

private:
	std::vector<plugin_loader> m_loaders;
	std::vector<std::reference_wrapper<const plugin>> m_plugins;
};

plugin_manager::plugin_manager() = default;

plugin_manager::plugin_manager(plugin_manager&& other) noexcept = default;

plugin_manager::~plugin_manager() = default;

plugin_manager&
plugin_manager::operator=(plugin_manager&& other) noexcept = default;

void plugin_manager::add_plugin(const plugin& plugin)
{
	create_if_null();
	m_implementation->add_plugin(plugin);
}

const plugin* plugin_manager::load_plugin(const std::string &path)
{
	create_if_null();
	return m_implementation->load_plugin(path);
}

std::size_t plugin_manager::get_plugin_count() const noexcept
{
	if (!m_implementation)
	{
		return 0;
	}
	else
	{
		return m_implementation->get_plugin_count();
	}
}

const plugin& plugin_manager::get_plugin(std::size_t index) const
{
	if (!m_implementation)
	{
		throw std::out_of_range("No plugins loaded");
	}
		return m_implementation->get_plugin(index);
}

void plugin_manager::create_if_null()
{
	if (!m_implementation)
	{
		m_implementation = std::make_unique<implementation>();
	}
}

std::string get_default_plugin_directory()
{
	// Address of any core function
	const auto* symbol = 
		reinterpret_cast<const void*>(&get_default_plugin_directory);

	auto path = boost::filesystem::path(
		dynamic_library::query_symbol_filename(symbol)
	);
	if(path.empty())
	{
		throw plugin_load_error(
			"Could not retrieve the default plugin directory"
		);
	}

	path.replace_filename(REXLIB_PLUGINS_DIRECTORY_NAME);
	return path.string();
}

#if REXLIB_WINDOWS
	static const char PATH_SEPARATOR = ';';
#else
	static const char PATH_SEPARATOR = ':';
#endif

static void append_unique(
	std::vector<std::string> &destination,
	std::string value
)
{
	const auto pos = std::find(destination.cbegin(), destination.cend(), value);
	if (pos == destination.cend())
	{
		destination.push_back(std::move(value));
	}
}

static void split_path(
	const std::string &path,
	std::vector<std::string> &destination
)
{
	std::string::size_type begin = 0;
	while (begin <= path.size())
	{
		auto end = path.find(PATH_SEPARATOR, begin);
		if (end == std::string::npos)
		{
			end = path.size();
		}

		// An empty entry carries no directory. Skipping it also means a
		// trailing or repeated separator is not read as the current
		// working directory, which is never what was meant.
		if (end > begin)
		{
			append_unique(destination, path.substr(begin, end - begin));
		}

		begin = end + 1;
	}
}

std::vector<std::string> get_plugin_search_path()
{
	std::vector<std::string> result;

	const char* environment_variable;
	if((environment_variable = std::getenv(REXLIB_PLUGINS_ENV_VARIABLE)))
	{
		split_path(environment_variable, result);
	}

	append_unique(result, get_default_plugin_directory());

	return result;
}

void discover_plugins(const std::string& directory, plugin_manager &manager)
{
	boost::filesystem::directory_iterator iterator;
	try
	{
		iterator = boost::filesystem::directory_iterator(directory);
	}
	catch(const boost::filesystem::filesystem_error& e)
	{
		REXLIB_LOG_DEBUG(
			"Failed to open plugin directory {}: {}", 
			directory, 
			e.what()
		);

		std::ignore = e; // To avoid compiler warnings with log disabled
	}

	for (const auto& entry : iterator) 
	{
		try_load_plugin(manager, entry.path().string());
	}
}

void discover_plugins(plugin_manager &manager)
{
	for (const auto &directory : get_plugin_search_path())
	{
		discover_plugins(directory, manager);
	}
}

std::size_t register_all_plugins_at(
	const plugin_manager &manager, 
	service_catalog &catalog
)
{
	const auto count = manager.get_plugin_count();
	for (std::size_t i = 0; i < count; ++i)
	{
		const auto &plugin = manager.get_plugin(i);
		plugin.register_at(catalog);
	}
	return count;
}

} // namespace rexlib
