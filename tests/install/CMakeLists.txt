# SPDX-License-Identifier: GPL-3.0-only

# Installs the library into a directory of the build tree, and then builds a
# program against what was installed and nothing else, as whoever installed
# rexlib would. A header that is not installed, or a package that does not
# describe the library, fails here rather than in their build.
set(REXLIB_INSTALL_TEST_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/prefix)

add_test(
	NAME rexlib-install
	COMMAND
		${CMAKE_COMMAND}
		--install ${PROJECT_BINARY_DIR}
		--prefix ${REXLIB_INSTALL_TEST_PREFIX}
		--config $<CONFIG>
)

# The consumer is given the compiler and the flags of this build, without
# which it would not link with a library built under a sanitizer. It is not
# given the toolchain file: the installed package has to be enough.
set(
	REXLIB_INSTALL_TEST_OPTIONS
	-DCMAKE_BUILD_TYPE=$<CONFIG>
	-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
	-DREXLIB_EXPECTED_VERSION=${PROJECT_VERSION}
	-Drexlib_ROOT=${REXLIB_INSTALL_TEST_PREFIX}
)

# Visual Studio chooses the compiler itself, and takes the platform apart.
set(REXLIB_INSTALL_TEST_GENERATOR --build-generator ${CMAKE_GENERATOR})
if(CMAKE_GENERATOR MATCHES "Visual Studio")
	if(CMAKE_GENERATOR_PLATFORM)
		list(
			APPEND REXLIB_INSTALL_TEST_GENERATOR
			--build-generator-platform ${CMAKE_GENERATOR_PLATFORM}
		)
	endif()
else()
	list(
		APPEND REXLIB_INSTALL_TEST_OPTIONS
		-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	)
endif()

# Windows finds a library beside the program or on the PATH, and the
# installed one is in neither place. So there the consumer is built and
# linked, and it is run everywhere else.
set(REXLIB_INSTALL_TEST_RUN)
if(NOT WIN32)
	set(
		REXLIB_INSTALL_TEST_RUN
		--test-command ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
	)
endif()

add_test(
	NAME rexlib-install-consumer
	COMMAND
		${CMAKE_CTEST_COMMAND}
		--build-and-test
			${CMAKE_CURRENT_SOURCE_DIR}/consumer
			${CMAKE_CURRENT_BINARY_DIR}/consumer
		${REXLIB_INSTALL_TEST_GENERATOR}
		--build-config $<CONFIG>
		--build-options ${REXLIB_INSTALL_TEST_OPTIONS}
		${REXLIB_INSTALL_TEST_RUN}
)

set_tests_properties(
	rexlib-install
	PROPERTIES
		FIXTURES_SETUP rexlib-installed
		LABELS install
)
set_tests_properties(
	rexlib-install-consumer
	PROPERTIES
		FIXTURES_REQUIRED rexlib-installed
		LABELS install
		TIMEOUT 3600
)
