# Add the test executable
add_executable(rexlib-unitary-tests)
add_subdirectory(src)
target_compile_definitions(
	rexlib-unitary-tests
	PRIVATE
		REXLIB_NO_EXPORTS
)
target_include_directories(
	rexlib-unitary-tests
	PRIVATE 
		${PROJECT_SOURCE_DIR}/src
)
# This suite reaches symbols the shared library hides, so it embeds the
# component objects rather than importing them. Linking the object libraries
# brings their private dependencies too, which is what the LINK_LIBRARIES of
# rexlib used to be read for.
#
# rexlib is linked as well, and on purpose: the plugins some cases load are
# modules against the shared library, and they have to resolve against it.
# This executable therefore holds a second copy of the library's state, hidden
# and distinct from the one those plugins see.
target_link_libraries(
	rexlib-unitary-tests
	PRIVATE
		rexlib-core-obj
		rexlib-ops-obj
		rexlib-functional-obj
		rexlib-cpu-obj
		rexlib-em-obj
		rexlib
		rexlib-test-assets-interface
		Catch2::Catch2WithMain
		trompeloeil::trompeloeil
)
add_dependencies(
	rexlib-unitary-tests
	rexlib-test-assets
)

# On Windows enable big objects
if(MSVC)
	target_compile_options(
		rexlib-unitary-tests
		PRIVATE
			/bigobj
	)
elseif(MINGW)
	target_compile_options(
		rexlib-unitary-tests
		PRIVATE
			-Wa,-mbig-obj
	)
endif()

if(MSVC)
	target_compile_options(
		rexlib-unitary-tests
		PRIVATE
			/Zc:preprocessor # HACK: Required for TROMPELOEIL_COUNT()
	)
endif()

# On Windows the plugin DLLs that some tests load (e.g. dynamic_library) depend
# on rexlib.dll. The loader resolves a plugin's imports from the test
# executable's directory, so copy the library next to it. Unlike the
# integration suite this executable embeds the core objects and never imports
# the DLL itself, hence the DLL would otherwise not be present here.
if(WIN32)
	add_custom_command(TARGET rexlib-unitary-tests
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:rexlib> $<TARGET_FILE_DIR:rexlib-unitary-tests>
	)
endif()

rexlib_enable_warnings(rexlib-unitary-tests)

# Register tests
rexlib_register_tests(rexlib-unitary-tests unitary)
