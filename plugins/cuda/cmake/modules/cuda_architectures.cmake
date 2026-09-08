cmake_minimum_required(VERSION 3.15) # list(POP_BACK)

# Reproduces the CMAKE_CUDA_ARCHITECTURES value "all-major" of CMake 3.23.
function(cuda_all_major_architectures out_var compiler)
	execute_process(
		COMMAND ${compiler} --list-gpu-arch
		OUTPUT_VARIABLE supported
		RESULT_VARIABLE status
		ERROR_QUIET
	)
	string(REGEX MATCHALL "compute_[0-9]+[a-z]*" supported "${supported}")

	# Accelerated variants such as compute_90a belong to a base architecture
	# instead of forming a family of their own.
	list(FILTER supported INCLUDE REGEX "^compute_[0-9]+$")
	list(TRANSFORM supported REPLACE "^compute_" "")

	if(NOT status EQUAL 0 OR NOT supported)
		message(FATAL_ERROR
			"${compiler} did not report the architectures it supports, "
			"which requires CUDA 11.1 or newer. Set "
			"CMAKE_CUDA_ARCHITECTURES explicitly to build with an older "
			"toolkit."
		)
	endif()

	# They are listed in ascending order, so the first one seen in a major
	# family is also the oldest that the toolkit still supports in it.
	set(architectures)
	set(families)
	foreach(architecture IN LISTS supported)
		math(EXPR family "${architecture} / 10")
		if(NOT family IN_LIST families)
			list(APPEND families ${family})
			list(APPEND architectures ${architecture})
		endif()
	endforeach()

	# All of them are embedded as SASS. The newest one additionally keeps its
	# PTX, which GPUs beyond it compile at load time.
	list(POP_BACK architectures newest)
	list(TRANSFORM architectures APPEND "-real")
	list(APPEND architectures ${newest})

	set(${out_var} "${architectures}" PARENT_SCOPE)
endfunction()
