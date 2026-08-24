// SPDX-License-Identifier: GPL-3.0-only

#include <rex/core/plugin.hpp>
#include <rex/core/platform/dynamic_shared_object.h>

#if defined(REXLIB_DUMMY_PLUGIN_EXPORTING)
	#define REXLIB_DUMMY_PLUGIN_API REXLIB_EXPORT
#else
	#define REXLIB_DUMMY_PLUGIN_API REXLIB_IMPORT
#endif

namespace rex
{

static const std::string name = "dummy-plugin";

class dummy_plugin final
	: public rex::plugin
{
	const std::string& get_name() const noexcept final
	{
		return name;
	}

	version get_version() const noexcept final
	{
		return version(1, 2, 3);
	}

	void register_at(service_catalog&) const
	{
		// NO-OP
	}
};

} // namespace rex

static const rex::dummy_plugin instance;

extern "C"
{
REXLIB_DUMMY_PLUGIN_API const rex::plugin* rex_get_plugin() 
{
	return &instance;
}
}
