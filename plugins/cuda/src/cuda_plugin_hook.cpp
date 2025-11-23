// SPDX-License-Identifier: GPL-3.0-only

#include "cuda_plugin.hpp"

#include <xmipp4/cuda/dynamic_shared_object.h>

static const xmipp4::cuda_plugin instance;

extern "C"
{
XMIPP4_CUDA_API const xmipp4::plugin* xmipp4_get_plugin() 
{
	return &instance;
}
}
