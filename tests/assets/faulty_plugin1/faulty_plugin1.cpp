// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#if defined(REXLIB_FAULTY_PLUGIN1_EXPORTING)
	#define REXLIB_FAULTY_PLUGIN1_API REXLIB_EXPORT
#else
	#define REXLIB_FAULTY_PLUGIN1_API REXLIB_IMPORT
#endif

extern "C"
{
REXLIB_FAULTY_PLUGIN1_API const rexlib::plugin* rexlib_get_plugin() 
{
	return NULL;
}
}
