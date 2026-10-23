target_sources(
	rexlib-core-obj
	PRIVATE
		operation.cpp
		operation_arity.cpp
		operation_cast.cpp
		operation_descriptor.cpp
)

add_subdirectory(contract)
