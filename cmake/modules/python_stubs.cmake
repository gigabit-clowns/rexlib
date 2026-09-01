cmake_minimum_required(VERSION 3.18)

# Installs type stubs for an extension module.
#
# Generating them means importing the extension, which a build that
# produces one it cannot run - a cross-compile - is unable to do. Passing
# STUBS_DIR installs stubs generated elsewhere instead; they describe the
# Python API, which does not vary by target.
function(install_python_stubs)
	set(oneValueArgs MODULE DESTINATION SEARCH_PATH STUBS_DIR GENERATOR)
	cmake_parse_arguments(PARSE_ARGV 0 arg "" "${oneValueArgs}" "")

	if(NOT arg_STUBS_DIR)
		install(CODE "
			execute_process(
				COMMAND \"${Python_EXECUTABLE}\" \"${arg_GENERATOR}\"
					--module \"${arg_MODULE}\"
					--search-path \"${arg_SEARCH_PATH}\"
					--output \"${arg_DESTINATION}\"
				RESULT_VARIABLE RESULT
			)
			if(NOT RESULT EQUAL 0)
				message(FATAL_ERROR \"Stub generation failed: \${RESULT}\")
			endif()
		")
		return()
	endif()

	get_filename_component(arg_STUBS_DIR
		"${arg_STUBS_DIR}" ABSOLUTE
		BASE_DIR "${PROJECT_SOURCE_DIR}"
	)

	# Shipping none of them would go unnoticed until somebody wondered why
	# their editor stopped helping.
	file(GLOB STUBS "${arg_STUBS_DIR}/*.pyi")
	if(NOT STUBS)
		message(FATAL_ERROR "No .pyi files in '${arg_STUBS_DIR}'")
	endif()

	install(
		DIRECTORY "${arg_STUBS_DIR}/"
		DESTINATION "${arg_DESTINATION}"
		FILES_MATCHING PATTERN "*.pyi"
	)
endfunction()
