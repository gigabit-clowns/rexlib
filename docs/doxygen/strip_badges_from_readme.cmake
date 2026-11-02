# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED INPUT_FILE)
	message(FATAL_ERROR "INPUT_FILE must be set")
endif()

if(NOT DEFINED OUTPUT_FILE)
	message(FATAL_ERROR "OUTPUT_FILE must be set")
endif()

file(STRINGS ${INPUT_FILE} INPUT_LINES)

set(OUTPUT_LINES "")
set(SKIP_BADGES OFF)

foreach(LINE IN LISTS INPUT_LINES)
	if(LINE MATCHES "^[ 	]*<!--[ 	]*docs-badges:start[ 	]*-->[ 	]*$")
		set(SKIP_BADGES ON)
		continue()
	endif()

	if(LINE MATCHES "^[ 	]*<!--[ 	]*docs-badges:end[ 	]*-->[ 	]*$")
		set(SKIP_BADGES OFF)
		continue()
	endif()

	if(SKIP_BADGES)
		continue()
	endif()

	if(LINE MATCHES "^[ 	]*\[!\[[^]]*\]\([^)]*badge[^)]*\)\]\([^)]*\)[ 	]*$")
		continue()
	endif()

	if(LINE MATCHES "^[ 	]*!\[[^]]*\]\([^)]*badge[^)]*\)[ 	]*$")
		continue()
	endif()

	list(APPEND OUTPUT_LINES "${LINE}")
endforeach()

string(REPLACE ";" "\n" OUTPUT_CONTENT "${OUTPUT_LINES}")
string(REGEX REPLACE "\n{3,}" "\n\n" OUTPUT_CONTENT "${OUTPUT_CONTENT}")
string(APPEND OUTPUT_CONTENT "\n")

file(WRITE ${OUTPUT_FILE} "${OUTPUT_CONTENT}")
