cmake_minimum_required(VERSION 3.12)

include(FetchContent)

function(fetch_spdlog)
	set(options)
	set(oneValueArgs VERSION)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	set(SPDLOG_INSTALL OFF)
	set(SPDLOG_BUILD_SHARED OFF)
	set(SPDLOG_BUILD_PIC ON)
	cmake_policy(SET CMP0135 NEW) # To avoid warnings
	FetchContent_Declare(
		spdlog
		URL https://github.com/gabime/spdlog/archive/refs/tags/v${arg_VERSION}.tar.gz
	)

	FetchContent_MakeAvailable(spdlog)
endfunction()
