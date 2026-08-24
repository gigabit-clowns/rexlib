// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpp_features.hpp"

#if REXLIB_HAS_CONDITIONAL_EXPLICIT
	#define REXLIB_NO_EXPLICIT explicit(false)
#else
	#define REXLIB_NO_EXPLICIT
#endif
