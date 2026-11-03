// SPDX-License-Identifier: GPL-3.0-only

// A shared object that calls a function no library defines.

extern "C" void rexlib_test_function_defined_nowhere();

extern "C" void rexlib_test_call_function_defined_nowhere()
{
	rexlib_test_function_defined_nowhere();
}
