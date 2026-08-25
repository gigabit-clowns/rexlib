cmake_minimum_required(VERSION 3.12)

include(FetchContent)

function(fetch_pocketfft)
	set(options)
	set(oneValueArgs COMMIT)
	set(multiValueArgs)
	cmake_parse_arguments(PARSE_ARGV 0 arg
		"${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	cmake_policy(SET CMP0135 NEW) # To avoid warnings
	FetchContent_Declare(
		pocketfft
		URL https://github.com/mreineck/pocketfft/archive/${arg_COMMIT}.tar.gz
	)
	FetchContent_MakeAvailable(pocketfft)

	set(pocketfft_INCLUDE_DIR ${pocketfft_SOURCE_DIR})

	# Define the target
	add_library(pocketfft INTERFACE)
	target_include_directories(
		pocketfft
		INTERFACE
			${pocketfft_INCLUDE_DIR}
	)
endfunction()
