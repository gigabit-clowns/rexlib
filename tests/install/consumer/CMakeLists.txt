# SPDX-License-Identifier: GPL-3.0-only

# A project of its own, as one that uses an installed rexlib is. It knows
# nothing of the tree it is in.
cmake_minimum_required(VERSION 3.18)

project(rexlib-install-consumer LANGUAGES CXX)

# The public headers are C++14.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(rexlib ${REXLIB_EXPECTED_VERSION} REQUIRED)

add_executable(rexlib-install-consumer main.cpp)

target_link_libraries(
	rexlib-install-consumer
	PRIVATE
		rexlib::rexlib
)

enable_testing()
add_test(NAME rexlib-install-consumer COMMAND rexlib-install-consumer)

# Windows finds a library beside the program or on the PATH, and the
# installed one is in neither place.
if(WIN32)
	string(REPLACE ";" "\\;" REXLIB_INSTALL_CONSUMER_PATH "$ENV{PATH}")
	set(
		REXLIB_INSTALL_CONSUMER_PATH
		"$<TARGET_FILE_DIR:rexlib::rexlib>\;${REXLIB_INSTALL_CONSUMER_PATH}"
	)
	set_tests_properties(
		rexlib-install-consumer
		PROPERTIES ENVIRONMENT "PATH=${REXLIB_INSTALL_CONSUMER_PATH}"
	)
endif()
