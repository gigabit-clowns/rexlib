cmake_minimum_required(VERSION 3.16)

include(CTest)
include(fetch_catch2)
include(fetch_trompeloeil)

fetch_catch2(VERSION 3.15.3)
fetch_trompeloeil(VERSION 49)

include(Catch)

function(rex_register_tests target)
	if(REXLIB_REGISTER_TESTS_PER_BINARY)
		add_test(NAME ${target} COMMAND ${target})
		set_tests_properties(${target} PROPERTIES TIMEOUT 3600)
	else()
		catch_discover_tests(${target})
	endif()
endfunction()

add_subdirectory(assets)
add_subdirectory(unitary)
add_subdirectory(integration)
