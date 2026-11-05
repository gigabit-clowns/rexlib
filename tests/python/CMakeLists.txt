# SPDX-License-Identifier: GPL-3.0-only

set(REXLIB_PYTHON_TEST_PLUGINS_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)
add_subdirectory(assets)

# The suite imports the package from an install into the build tree, which
# is everything this build installs.
set(REXLIB_PYTHON_TEST_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/prefix)

add_test(
	NAME rexlib-python-install
	COMMAND
		${CMAKE_COMMAND}
		--install ${CMAKE_BINARY_DIR}
		--prefix ${REXLIB_PYTHON_TEST_PREFIX}
		--config $<CONFIG>
)
set_tests_properties(
	rexlib-python-install
	PROPERTIES
		FIXTURES_SETUP rexlib-python-installed
		LABELS python
)

add_test(
	NAME rexlib-python
	COMMAND
		${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}
		-p no:cacheprovider
)
set_tests_properties(
	rexlib-python
	PROPERTIES
		FIXTURES_REQUIRED rexlib-python-installed
		LABELS python
		ENVIRONMENT
			"PYTHONPATH=${REXLIB_PYTHON_TEST_PREFIX}/${REXLIB_PYTHON_INSTALL_DIR};\
REXLIB_PYTHON_TEST_PLUGINS_DIR=${REXLIB_PYTHON_TEST_PLUGINS_DIR}"
)

set_property(
	GLOBAL APPEND PROPERTY REXLIB_MEMCHECK_IGNORE
	rexlib-python-install
	rexlib-python
)
