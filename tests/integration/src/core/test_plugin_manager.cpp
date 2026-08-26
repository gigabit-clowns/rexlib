// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/plugin_manager.hpp>

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/exceptions/plugin_load_error.hpp>

#include <rexlib/tests/assets.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <cstdlib>

using namespace rexlib;

#if REXLIB_WINDOWS
	static const char PATH_SEPARATOR[] = ";";
#else
	static const char PATH_SEPARATOR[] = ":";
#endif

/**
 * @brief Sets REXLIB_PLUGINS_PATH for the duration of a test case.
 * 
 * The variable is read on every call, so it has to be restored before
 * the next test case runs.
 * 
 */
class scoped_plugins_path
{
public:
	scoped_plugins_path()
	{
		save();
		unset();
	}
	explicit scoped_plugins_path(const std::string &value)
	{
		save();
		set(value.c_str());
	}
	scoped_plugins_path(const scoped_plugins_path&) = delete;
	scoped_plugins_path& operator=(const scoped_plugins_path&) = delete;

	~scoped_plugins_path()
	{
		if (m_had_previous)
		{
			set(m_previous.c_str());
		}
		else
		{
			unset();
		}
	}

private:
	static constexpr const char* NAME = "REXLIB_PLUGINS_PATH";

	std::string m_previous;
	bool m_had_previous {false};

	void save()
	{
		const char* previous = std::getenv(NAME);
		if (previous)
		{
			m_previous = previous;
			m_had_previous = true;
		}
	}

	static void set(const char* value)
	{
		#if REXLIB_WINDOWS
			_putenv_s(NAME, value);
		#else
			setenv(NAME, value, 1);
		#endif
	}

	static void unset()
	{
		#if REXLIB_WINDOWS
			_putenv_s(NAME, "");
		#else
			unsetenv(NAME);
		#endif
	}
};


TEST_CASE( "load good plugin in the plugin manager", "[plugin_manager]" ) 
{
	plugin_manager manager;
	const auto* plugin = manager.load_plugin(get_mock_plugin_path("dummy-plugin"));
	REQUIRE( plugin->get_name() == "dummy-plugin" );
	REQUIRE( plugin->get_version() == version(1, 2, 3) );
	REQUIRE( manager.get_plugin_count() == 1 );
	REQUIRE( &(manager.get_plugin(0)) == plugin );
}

TEST_CASE( "load invalid plugin in the plugin manager should throw", "[plugin_manager]" ) 
{
	plugin_manager manager;

	REQUIRE_THROWS_AS(
		manager.load_plugin("path_to_nowhere"), 
		std::system_error
	);
	REQUIRE( manager.get_plugin_count() == 0 );

	REQUIRE_THROWS_AS( 
		manager.load_plugin(get_text_file_path()), 
		std::system_error
	);
	REQUIRE( manager.get_plugin_count() == 0 );

	REQUIRE_THROWS_MATCHES( 
		manager.load_plugin(get_mock_plugin_path("faulty-plugin1")),
		plugin_load_error,
		Catch::Matchers::Message("rexlib_get_plugin returned NULL.")
	);
	REQUIRE( manager.get_plugin_count() == 0 );

	REQUIRE_THROWS_MATCHES( 
		manager.load_plugin(get_mock_plugin_path("faulty-plugin2")),
		plugin_load_error,
		Catch::Matchers::Message("rexlib_get_plugin symbol could not be found in shared object.")
	);
	REQUIRE( manager.get_plugin_count() == 0 );
}

TEST_CASE( "querying out out range plugin from plugin manager should throw", "[plugin_manager]" ) 
{
	plugin_manager manager;
	manager.load_plugin(get_mock_plugin_path("dummy-plugin"));
	REQUIRE_THROWS_AS( manager.get_plugin(1), std::out_of_range );
	REQUIRE_THROWS_AS( manager.get_plugin(10), std::out_of_range );
}

TEST_CASE( "discover_plugins should tolerate invalid plugins", "[plugin_manager]" ) 
{
	plugin_manager manager;
	REQUIRE_NOTHROW( discover_plugins(get_test_plugin_directory(), manager) );
	REQUIRE( manager.get_plugin_count() == 1 );

	const plugin& plugin = manager.get_plugin(0);
	REQUIRE( plugin.get_name() == "dummy-plugin" );
	REQUIRE( plugin.get_version() == version(1, 2, 3) );
}

TEST_CASE( "discover_plugins should tolerate non-existing directories", "[plugin_manager]" ) 
{
	plugin_manager manager;
	REQUIRE_NOTHROW( discover_plugins("/path/to/nowhere/dshfjbfnmxbcusdfj", manager) );
	REQUIRE( manager.get_plugin_count() == 0 );
}

TEST_CASE(
	"get_plugin_search_path should end with the default directory",
	"[plugin_manager]"
)
{
	const scoped_plugins_path guard;

	const auto path = get_plugin_search_path();
	REQUIRE( path.size() == 1 );
	REQUIRE( path.back() == get_default_plugin_directory() );
}

TEST_CASE(
	"get_plugin_search_path should prepend the environment entries",
	"[plugin_manager]"
)
{
	const scoped_plugins_path guard(
		std::string("/first") + PATH_SEPARATOR + "/second"
	);

	const auto path = get_plugin_search_path();
	REQUIRE( path.size() == 3 );
	REQUIRE( path[0] == "/first" );
	REQUIRE( path[1] == "/second" );
	REQUIRE( path[2] == get_default_plugin_directory() );
}

TEST_CASE(
	"get_plugin_search_path should drop empty entries",
	"[plugin_manager]"
)
{
	const scoped_plugins_path guard(
		std::string(PATH_SEPARATOR) + PATH_SEPARATOR + "/only" +
		PATH_SEPARATOR + PATH_SEPARATOR
	);

	const auto path = get_plugin_search_path();
	REQUIRE( path.size() == 2 );
	REQUIRE( path[0] == "/only" );
	REQUIRE( path[1] == get_default_plugin_directory() );
}

TEST_CASE(
	"get_plugin_search_path should remove duplicates",
	"[plugin_manager]"
)
{
	const scoped_plugins_path guard(
		std::string("/twice") + PATH_SEPARATOR + "/twice" +
		PATH_SEPARATOR + get_default_plugin_directory()
	);

	const auto path = get_plugin_search_path();
	REQUIRE( path.size() == 2 );
	REQUIRE( path[0] == "/twice" );
	REQUIRE( path[1] == get_default_plugin_directory() );
}

TEST_CASE(
	"discover_plugins should search every entry of the path",
	"[plugin_manager]"
)
{
	const scoped_plugins_path guard(get_test_plugin_directory());

	plugin_manager manager;
	REQUIRE_NOTHROW( discover_plugins(manager) );
	REQUIRE( manager.get_plugin_count() == 1 );
	REQUIRE( manager.get_plugin(0).get_name() == "dummy-plugin" );
}
