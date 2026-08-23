// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/plugin.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

#if defined(REX_FAULTY_PLUGIN1_EXPORTING)
	#define REX_FAULTY_PLUGIN1_API REX_EXPORT
#else
	#define REX_FAULTY_PLUGIN1_API REX_IMPORT
#endif

extern "C"
{
REX_FAULTY_PLUGIN1_API const rex::plugin* rex_get_plugin() 
{
	return NULL;
}
}
