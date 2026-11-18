# SPDX-License-Identifier: GPL-3.0-only

# Declared here rather than beside the other toggles, because these two
# dependencies exist only while the tests are built.
cmake_dependent_option(
	REXLIB_CUDA_USE_SYSTEM_CATCH2
	"Find an installed Catch2 rather than fetching it"
	OFF
	"NOT REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES"
	ON
)
cmake_dependent_option(
	REXLIB_CUDA_USE_SYSTEM_TROMPELOEIL
	"Find an installed trompeloeil rather than fetching it"
	OFF
	"NOT REXLIB_CUDA_USE_SYSTEM_DEPENDENCIES"
	ON
)

if(REXLIB_CUDA_USE_SYSTEM_CATCH2)
	find_package(Catch2 3 REQUIRED CONFIG)
else()
	include(rexlib_cuda_add_catch2)
	rexlib_cuda_add_catch2(SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/catch2)
endif()

if(REXLIB_CUDA_USE_SYSTEM_TROMPELOEIL)
	find_package(trompeloeil REQUIRED CONFIG)
else()
	include(rexlib_cuda_add_trompeloeil)
	rexlib_cuda_add_trompeloeil(
		SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/trompeloeil
	)
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
function(rexlib_cuda_register_tests TARGET LABEL)
	if(REXLIB_CUDA_REGISTER_TESTS_PER_BINARY OR CMAKE_VERSION VERSION_LESS 3.19)
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

add_subdirectory(unitary)
add_subdirectory(integration)
