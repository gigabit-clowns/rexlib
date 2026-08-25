// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "pragma.h"

/**
 * @def REXLIB_OMP
 * @brief Provide a OpenMP directive to the compiler.
 * 
 */
#if _OPENMP
	#define REXLIB_OMP(x) REXLIB_PRAGMA(omp x)
#else
	#define REXLIB_OMP(x)
#endif
