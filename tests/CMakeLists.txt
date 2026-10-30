# Declared here rather than beside the other toggles, because these two
# dependencies exist only while the tests are built.
cmake_dependent_option(
	REXLIB_BUILD_CATCH2 "Fetch Catch2 and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)
cmake_dependent_option(
	REXLIB_BUILD_TROMPELOEIL "Fetch trompeloeil and build it with rexlib" ON
	"REXLIB_BUILD_DEPENDENCIES" OFF
)

if(REXLIB_BUILD_CATCH2)
	include(rexlib_fetch_catch2)
	rexlib_fetch_catch2(
		COMMIT 08092139210881f01a34e60c5f2fd6afa0ea4024
	) # v3.16.1
else()
	find_package(Catch2 3.0.1 REQUIRED CONFIG)
endif()

if(REXLIB_BUILD_TROMPELOEIL)
	include(rexlib_fetch_trompeloeil)
	rexlib_fetch_trompeloeil(
		COMMIT eaeb89c1ce9d354b0ba7eb921fd5712cdbd78adf
	) # v49
else()
	find_package(trompeloeil 42 REQUIRED CONFIG)
endif()

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
