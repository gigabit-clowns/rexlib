# SPDX-License-Identifier: GPL-3.0-only

# nanobind falls back to the interpreter's own ABI where the stable one
# cannot serve: before Python 3.12, and on a free-threaded interpreter.
set(NANOBIND_OPTIONS NB_SUPPRESS_WARNINGS)
if(REXLIB_PYTHON_STABLE_ABI)
	list(APPEND NANOBIND_OPTIONS STABLE_ABI)
endif()
nanobind_add_module(_binding ${NANOBIND_OPTIONS})
target_sources(
	_binding
	PRIVATE
		main.cpp
)
add_subdirectory(core)
add_subdirectory(em)
add_subdirectory(functional)

target_compile_definitions(
	_binding
	PRIVATE
		REXLIB_PYTHON_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
		REXLIB_PYTHON_VERSION_MINOR=${PROJECT_VERSION_MINOR}
		REXLIB_PYTHON_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# librexlib ships in the package's own library directory, whichever name
# the platform gives it, so the loader resolves it from the extension's own
# location. Windows needs nothing: the DLL sits beside the .pyd and CPython
# loads extensions with LOAD_WITH_ALTERED_SEARCH_PATH.
if(APPLE)
	set_target_properties(
		_binding
		PROPERTIES
			INSTALL_RPATH "@loader_path/${CMAKE_INSTALL_LIBDIR}"
	)
elseif(UNIX)
	set_target_properties(
		_binding
		PROPERTIES
			INSTALL_RPATH "$ORIGIN/${CMAKE_INSTALL_LIBDIR}"
	)
endif()

# A package that carries no rexlib also looks where this build found it.
if(UNIX AND NOT REXLIB_PYTHON_STAGE_PREFIX)
	get_target_property(REXLIB_IMPORTED rexlib::rexlib IMPORTED)
	if(REXLIB_IMPORTED)
		get_target_property(REXLIB_LOCATION rexlib::rexlib LOCATION)
		get_filename_component(REXLIB_LIBRARY_DIR "${REXLIB_LOCATION}" DIRECTORY)
		set_property(
			TARGET _binding
			APPEND PROPERTY INSTALL_RPATH "${REXLIB_LIBRARY_DIR}"
		)
	endif()
endif()

target_link_libraries(
	_binding
	PRIVATE
		rexlib::rexlib
		dlpack::dlpack
)
# std::complex is specified for float, double and long double only, and the
# standard library of MSVC says so wherever it is used with anything else.
if(MSVC)
	target_compile_definitions(
		_binding
		PRIVATE
			_SILENCE_NONFLOATING_COMPLEX_DEPRECATION_WARNING
	)
endif()
rexlib_python_enable_warnings(_binding)

if(REXLIB_PYTHON_INSTALL)
	install(
		TARGETS _binding
		LIBRARY DESTINATION "${REXLIB_PYTHON_PACKAGE_DIR}"
	)
endif()
