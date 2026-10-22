include(rexlib_add_catch2)
include(rexlib_add_trompeloeil)

# Declared here rather than beside the other REXLIB_USE_SYSTEM_* options,
# because these two dependencies exist only while the tests are built. The
# global switch reaches them the same way it reaches the rest.
option(REXLIB_USE_SYSTEM_CATCH2 "Use find_package for Catch2" OFF)
option(REXLIB_USE_SYSTEM_TROMPELOEIL "Use find_package for trompeloeil" OFF)

if(REXLIB_USE_SYSTEM_DEPENDENCIES)
	set(REXLIB_USE_SYSTEM_CATCH2 ON)
	set(REXLIB_USE_SYSTEM_TROMPELOEIL ON)
endif()

rexlib_add_catch2(VERSION 3.16.1 MINIMUM 3.0.1)
rexlib_add_trompeloeil(VERSION 49 MINIMUM 42)

include(Catch)

# The label goes on either registration, so that ctest -L selects a suite
# whichever way the cases were registered.
function(rexlib_register_tests target label)
	if(REXLIB_REGISTER_TESTS_PER_BINARY)
		add_test(NAME ${target} COMMAND ${target})
		set_tests_properties(
			${target}
			PROPERTIES
				TIMEOUT 3600
				LABELS ${label}
		)
	else()
		catch_discover_tests(${target} PROPERTIES LABELS ${label})
	endif()
endfunction()

add_subdirectory(assets)
add_subdirectory(unitary)
add_subdirectory(integration)
