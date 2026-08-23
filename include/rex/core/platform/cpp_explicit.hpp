// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpp_features.hpp"

#if REX_HAS_CONDITIONAL_EXPLICIT
	#define REX_NO_EXPLICIT explicit(false)
#else
	#define REX_NO_EXPLICIT
#endif
