// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpp_features.hpp"

#if REXLIB_HAS_LIB_EXECUTION
	#include <execution>
	#define REXLIB_SEQ std::execution::seq,
	#define REXLIB_PAR std::execution::par,
	#define REXLIB_PAR_UNSEQ std::execution::par_unseq,
#else
	#define REXLIB_SEQ
	#define REXLIB_PAR
	#define REXLIB_PAR_UNSEQ
#endif
