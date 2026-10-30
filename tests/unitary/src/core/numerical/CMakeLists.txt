# SPDX-License-Identifier: GPL-3.0-only

target_sources(
	rexlib-unitary-tests
	PRIVATE
		test_fixed_width_float.cpp
		test_float16_conversions.cpp
		test_numerical_cast.cpp
		test_numerical_type.cpp
		test_numerical_type_dispatch.cpp
		test_numerical_type_domain.cpp
		test_numerical_type_traits.cpp
		test_scalar_value.cpp
)
