# SPDX-License-Identifier: GPL-3.0-only

# Not registered with CTest: a benchmark wants the machine to itself and is
# run by hand.
add_executable(rexlib-benchmarks)
add_subdirectory(src)
target_compile_definitions(
	rexlib-benchmarks
	PRIVATE
		REXLIB_NO_EXPORTS
)
target_include_directories(
	rexlib-benchmarks
	PRIVATE
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/tests/unitary/src
)
# As the unitary suite does, it embeds the component objects to reach the
# internals of the CPU backend.
target_link_libraries(
	rexlib-benchmarks
	PRIVATE
		rexlib-core-obj
		rexlib-ops-obj
		rexlib-functional-obj
		rexlib-cpu-obj
		rexlib-em-obj
		rexlib
		Catch2::Catch2WithMain
)

rexlib_enable_warnings(rexlib-benchmarks)
