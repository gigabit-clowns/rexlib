# SPDX-License-Identifier: GPL-3.0-only

# The dependency provider of the CUDA plugin's build: every find_package() of a
# package it knows, from the plugin or its tests, gets that package built from
# its sources in plugins/cuda/third_party/, added once, at the first call. Any
# other find_package() searches as usual.
#
# It takes effect given to the first project() through
# CMAKE_PROJECT_TOP_LEVEL_INCLUDES, as the presets and the wheels'
# pyproject.toml do, which needs CMake 3.24. Without it, every dependency but
# rexlib is found installed.
#
# A call stops the configure when the version it asks for is newer than the
# one provided, or when a component it requires has no <package>::<component>
# target.

if(CMAKE_VERSION VERSION_LESS 3.24)
	message(FATAL_ERROR "The CUDA plugin's dependency provider needs CMake 3.24")
endif()

get_filename_component(
	rexlib_cuda_third_party_dir "${CMAKE_CURRENT_LIST_DIR}/../third_party" ABSOLUTE
)
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDER_THIRD_PARTY_DIR
	"${rexlib_cuda_third_party_dir}"
)
unset(rexlib_cuda_third_party_dir)
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDER_MODULE_DIR
	"${CMAKE_CURRENT_LIST_DIR}/modules"
)

# Each package provided: the module that adds it, and its directory in
# plugins/cuda/third_party/.
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDES_Boost rexlib_cuda_add_boost boost
)
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDES_spdlog rexlib_cuda_add_spdlog spdlog
)
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDES_Catch2 rexlib_cuda_add_catch2 catch2
)
set_property(
	GLOBAL PROPERTY REXLIB_CUDA_PROVIDES_trompeloeil
	rexlib_cuda_add_trompeloeil trompeloeil
)

function(rexlib_cuda_provide_dependency method package)
	get_property(entry GLOBAL PROPERTY REXLIB_CUDA_PROVIDES_${package})
	if(NOT entry)
		return()
	endif()
	list(GET entry 0 module)
	list(GET entry 1 directory)
	get_property(third_party GLOBAL PROPERTY REXLIB_CUDA_PROVIDER_THIRD_PARTY_DIR)
	set(source_dir "${third_party}/${directory}")
	set(caller_module_path "${CMAKE_MODULE_PATH}")

	get_property(provided GLOBAL PROPERTY REXLIB_CUDA_PROVIDED_${package} SET)
	if(NOT provided)
		get_property(module_dir GLOBAL PROPERTY REXLIB_CUDA_PROVIDER_MODULE_DIR)
		list(APPEND CMAKE_MODULE_PATH "${module_dir}")
		set(module_path "${CMAKE_MODULE_PATH}")
		include(${module})
		cmake_language(CALL ${module} SOURCE_DIR "${source_dir}")

		set(added_module_path "${CMAKE_MODULE_PATH}")
		list(REMOVE_ITEM added_module_path ${module_path})
		set_property(
			GLOBAL PROPERTY REXLIB_CUDA_PROVIDED_${package} "${added_module_path}"
		)
	endif()

	get_property(
		version GLOBAL PROPERTY REXLIB_CUDA_DEPENDENCY_VERSION_${source_dir}
	)
	cmake_parse_arguments(arg "" "" "COMPONENTS" ${ARGN})
	list(GET ARGN 0 requested)
	if(requested MATCHES "^[0-9]" AND version AND version VERSION_LESS requested)
		message(
			FATAL_ERROR
			"${CMAKE_CURRENT_SOURCE_DIR} asks for ${package} ${requested}, "
			"newer than the ${version} in ${source_dir}"
		)
	endif()
	foreach(component IN LISTS arg_COMPONENTS)
		if(NOT TARGET ${package}::${component})
			message(
				FATAL_ERROR
				"${CMAKE_CURRENT_SOURCE_DIR} asks for the ${component} component "
				"of ${package}, which ${source_dir} does not provide"
			)
		endif()
	endforeach()

	get_property(added_module_path GLOBAL PROPERTY REXLIB_CUDA_PROVIDED_${package})
	set(${package}_FOUND TRUE PARENT_SCOPE)
	set(${package}_VERSION "${version}" PARENT_SCOPE)
	set(
		CMAKE_MODULE_PATH "${caller_module_path};${added_module_path}"
		PARENT_SCOPE
	)
endfunction()

cmake_language(
	SET_DEPENDENCY_PROVIDER rexlib_cuda_provide_dependency
	SUPPORTED_METHODS FIND_PACKAGE
)
