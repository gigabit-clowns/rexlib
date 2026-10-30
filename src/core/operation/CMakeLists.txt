# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-core-obj
	PRIVATE
		operand_specification.cpp
		operation.cpp
		operation_arity.cpp
		operation_cast.cpp
		operation_descriptor.cpp
		operation_signature.cpp
)

add_subdirectory(shape_rule)
add_subdirectory(type_rule)
