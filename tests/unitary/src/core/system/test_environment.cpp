// SPDX-License-Identifier: GPL-3.0-only

#include <core/system/environment.hpp>

#include <rexlib/core/platform/operating_system.h>

#if REXLIB_WINDOWS
	#include <core/system/utf16_windows.hpp>
#endif

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <string>

using namespace rexlib;

static const char *const variable_name = "REXLIB_TEST_ENVIRONMENT_VARIABLE";

static void set_variable(const std::string &value)
{
	#if REXLIB_WINDOWS
		::_wputenv_s(
			utf8_to_utf16(variable_name).c_str(),
			utf8_to_utf16(value).c_str()
		);
	#else
		::setenv(variable_name, value.c_str(), 1);
	#endif
}

static void unset_variable()
{
	#if REXLIB_WINDOWS
		::_wputenv_s(utf8_to_utf16(variable_name).c_str(), L"");
	#else
		::unsetenv(variable_name);
	#endif
}

TEST_CASE(
	"get_environment_variable should return the value in UTF-8",
	"[environment]"
)
{
	const std::string value = "/plugins-\xc3\xb1";
	set_variable(value);

	CHECK( get_environment_variable(variable_name) == value );

	unset_variable();
}

TEST_CASE(
	"get_environment_variable should return nothing for an unset variable",
	"[environment]"
)
{
	unset_variable();

	CHECK( get_environment_variable(variable_name).empty() );
}
