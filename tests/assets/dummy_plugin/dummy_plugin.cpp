// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(REX_DUMMY_PLUGIN_EXPORTING)
	#define REX_DUMMY_PLUGIN_API REX_EXPORT
#else
	#define REX_DUMMY_PLUGIN_API REX_IMPORT
#endif

namespace rex
{

static const std::string name = "dummy-plugin";

class dummy_plugin final
	: public xmipp4::plugin
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

static const xmipp4::dummy_plugin instance;

extern "C"
{
REX_DUMMY_PLUGIN_API const xmipp4::plugin* xmipp4_get_plugin() 
{
	return &instance;
}
}
