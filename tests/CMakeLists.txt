# SPDX-License-Identifier: GPL-3.0-only

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

# Registers the cases of a test executable with CTest, either one by one or
# as a single test that runs them all. The single test is what a run under a
# slow tool wants, since it starts the executable once. The label goes on
# either registration, so that ctest -L selects a suite whichever way its
# cases were registered.
#
# Catch2 lists the cases through CMake's JSON support, which CMake 3.18 does
# not have, so there the single test is the only choice.
function(rexlib_register_tests TARGET LABEL)
	if(REXLIB_REGISTER_TESTS_PER_BINARY OR CMAKE_VERSION VERSION_LESS 3.19)
		add_test(NAME ${TARGET} COMMAND ${TARGET})
		set_tests_properties(
			${TARGET}
			PROPERTIES
				TIMEOUT 3600
				LABELS ${LABEL}
		)
	else()
		catch_discover_tests(${TARGET} PROPERTIES LABELS ${LABEL})
	endif()
endfunction()

add_subdirectory(assets)
add_subdirectory(unitary)
add_subdirectory(integration)
