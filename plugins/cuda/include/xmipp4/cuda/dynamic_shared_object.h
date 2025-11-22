// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(XMIPP4_HARDWARE_CUDA_EXPORTING)
	#define XMIPP4_HARDWARE_CUDA_API XMIPP4_EXPORT
#else
	#define XMIPP4_HARDWARE_CUDA_API XMIPP4_IMPORT
#endif
