cmake_minimum_required(VERSION 3.12)

include(FetchContent)

function(fetch_boost)
	set(options)
	set(oneValueArgs VERSION)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	cmake_policy(SET CMP0135 NEW) # To avoid warnings
	FetchContent_Declare(
		Boost
		URL https://github.com/boostorg/boost/releases/download/boost-${arg_VERSION}/boost-${arg_VERSION}-cmake.tar.gz
	)

	set(BOOST_INCLUDE_LIBRARIES
		container
		intrusive
		unordered
	)
	set(BOOST_CONTAINER_HEADER_ONLY ON)
	set(BOOST_USE_STATIC_LIBS ON)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	FetchContent_MakeAvailable(Boost)

endfunction()
