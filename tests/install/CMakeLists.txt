# SPDX-License-Identifier: GPL-3.0-only

# Installs the library into a directory of the build tree, then builds a
# plugin and a program against what was installed and nothing else, as
# whoever installed rexlib would. The plugin is installed beside the library,
# and the program loads it and runs its program. A header that is not
# installed, a package that does not describe the library, or a plugin SDK
# that is not enough fails here rather than in their build.
set(REXLIB_INSTALL_TEST_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/prefix)

add_test(
	NAME rexlib-install
	COMMAND
		${CMAKE_COMMAND}
		--install ${PROJECT_BINARY_DIR}
		--prefix ${REXLIB_INSTALL_TEST_PREFIX}
		--config $<CONFIG>
)

# The plugin and the consumer are given the compiler and the flags of this
# build, without which they would not link with a library built under a
# sanitizer. They are not given the toolchain file: the installed package has
# to be enough.
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

add_test(
	NAME rexlib-install-plugin-build
	COMMAND
		${CMAKE_CTEST_COMMAND}
		--build-and-test
			${CMAKE_CURRENT_SOURCE_DIR}/plugin
			${CMAKE_CURRENT_BINARY_DIR}/plugin
		${REXLIB_INSTALL_TEST_GENERATOR}
		--build-config $<CONFIG>
		--build-options ${REXLIB_INSTALL_TEST_OPTIONS}
)

add_test(
	NAME rexlib-install-plugin
	COMMAND
		${CMAKE_COMMAND}
		--install ${CMAKE_CURRENT_BINARY_DIR}/plugin
		--prefix ${REXLIB_INSTALL_TEST_PREFIX}
		--config $<CONFIG>
)

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
		--test-command ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --output-on-failure
)

# Under ctest -T memcheck these would check cmake and ctest, not rexlib, and
# a memory checker reports what those keep until they exit.
set_property(
	GLOBAL APPEND PROPERTY REXLIB_MEMCHECK_IGNORE
	rexlib-install
	rexlib-install-plugin-build
	rexlib-install-plugin
	rexlib-install-consumer
)

set_tests_properties(
	rexlib-install
	PROPERTIES
		FIXTURES_SETUP rexlib-installed
		LABELS install
)
set_tests_properties(
	rexlib-install-plugin-build
	PROPERTIES
		FIXTURES_REQUIRED rexlib-installed
		FIXTURES_SETUP rexlib-install-plugin-built
		LABELS install
		TIMEOUT 3600
)
set_tests_properties(
	rexlib-install-plugin
	PROPERTIES
		FIXTURES_REQUIRED rexlib-install-plugin-built
		FIXTURES_SETUP rexlib-install-plugin-installed
		LABELS install
)
set_tests_properties(
	rexlib-install-consumer
	PROPERTIES
		FIXTURES_REQUIRED "rexlib-installed;rexlib-install-plugin-installed"
		LABELS install
		TIMEOUT 3600
)
