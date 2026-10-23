target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_basic_operation.cpp
		test_operation.cpp
		test_operation_arity.cpp
		test_operation_cast.cpp
		test_operation_id.cpp
		test_parametric_operation.cpp
		test_trivial_operation.cpp
)

add_subdirectory(contract)
