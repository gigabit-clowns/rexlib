// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cpp_features.hpp"

#if REX_HAS_LIB_EXECUTION
	#include <execution>
	#define REX_SEQ std::execution::seq,
	#define REX_PAR std::execution::par,
	#define REX_PAR_UNSEQ std::execution::par_unseq,
#else
	#define REX_SEQ
	#define REX_PAR
	#define REX_PAR_UNSEQ
#endif
