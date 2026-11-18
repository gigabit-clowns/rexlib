# SPDX-License-Identifier: GPL-3.0-only

# Compiles every public header on its own, once per C++ standard the compiler
# supports. A header that misses an include, or that uses something newer
# than C++14 without a feature macro around it, fails here rather than in a
# consumer.
#
# The headers are globbed, unlike the sources of the library, so that a new
# header is checked without anyone remembering to list it. Those CMake writes
# are in the build tree.
set(REXLIB_PUBLIC_HEADERS)
foreach(
	INCLUDE_DIR
	${PROJECT_SOURCE_DIR}/include
	${REXLIB_GENERATED_INCLUDE_DIR}
)
	file(
		GLOB_RECURSE HEADERS
		CONFIGURE_DEPENDS
		RELATIVE ${INCLUDE_DIR}
		${INCLUDE_DIR}/*.hpp
		${INCLUDE_DIR}/*.h
	)
	list(APPEND REXLIB_PUBLIC_HEADERS ${HEADERS})
endforeach()

foreach(STANDARD 14 17 20 23)
	if(NOT "cxx_std_${STANDARD}" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
		continue()
	endif()

	set(TARGET rexlib-header-check-cxx${STANDARD})
	set(SOURCES)
	foreach(HEADER ${REXLIB_PUBLIC_HEADERS})
		string(MAKE_C_IDENTIFIER ${HEADER} NAME)
		set(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/cxx${STANDARD}/${NAME}.cpp)
		set(CONTENT "#include <${HEADER}>\n")

		# Written only when it changes, so that configuring again does not
		# compile every header again.
		set(EXISTING "")
		if(EXISTS ${SOURCE})
			file(READ ${SOURCE} EXISTING)
		endif()
		if(NOT EXISTING STREQUAL CONTENT)
			file(WRITE ${SOURCE} "${CONTENT}")
		endif()

		list(APPEND SOURCES ${SOURCE})
	endforeach()

	add_library(${TARGET} OBJECT ${SOURCES})
	target_link_libraries(${TARGET} PRIVATE rexlib)
	set_target_properties(
		${TARGET}
		PROPERTIES
			CXX_STANDARD ${STANDARD}
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
	)
	rexlib_enable_warnings(${TARGET})
endforeach()
