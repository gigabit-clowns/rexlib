// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(REX_FAULTY_PLUGIN1_EXPORTING)
	#define REX_FAULTY_PLUGIN1_API REX_EXPORT
#else
	#define REX_FAULTY_PLUGIN1_API REX_IMPORT
#endif

extern "C"
{
REX_FAULTY_PLUGIN1_API const xmipp4::plugin* xmipp4_get_plugin() 
{
	return NULL;
}
}
