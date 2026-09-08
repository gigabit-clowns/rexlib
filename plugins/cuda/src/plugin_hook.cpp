// SPDX-License-Identifier: GPL-3.0-only

#include "plugin.hpp"

#include <rexlib/core/platform/dynamic_shared_object.h>

#if defined(REXLIB_CUDA_EXPORTING)
	#define REXLIB_CUDA_API REXLIB_EXPORT
#else
	#define REXLIB_CUDA_API REXLIB_IMPORT
#endif

static const rexlib::cuda_plugin instance;

extern "C"
{
REXLIB_CUDA_API const rexlib::plugin* rexlib_get_plugin()
{
	return &instance;
}
}
