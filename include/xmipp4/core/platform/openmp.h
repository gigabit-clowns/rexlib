// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "pragma.h"

/**
 * @def REX_OMP
 * @brief Provide a OpenMP directive to the compiler.
 * 
 */
#if _OPENMP
	#define REX_OMP(x) REX_PRAGMA(omp x)
#else
	#define REX_OMP(x)
#endif
