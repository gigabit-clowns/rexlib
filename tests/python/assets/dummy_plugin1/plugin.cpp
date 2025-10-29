// SPDX-License-Identifier: GPL-3.0-only

#include <xmipp4/core/plugin.hpp>
#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(XMIPP4_DUMMY_PLUGIN1_EXPORTING)
    #define XMIPP4_DUMMY_PLUGIN1_API XMIPP4_EXPORT
#else
    #define XMIPP4_DUMMY_PLUGIN1_API XMIPP4_IMPORT
#endif

namespace xmipp4
{

static const std::string name = "dummy-plugin1";

class dummy_plugin1 final
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

} // namespace xmipp4

static const xmipp4::dummy_plugin1 instance;

extern "C"
{
XMIPP4_DUMMY_PLUGIN1_API const xmipp4::plugin* xmipp4_get_plugin() 
{
    return &instance;
}
}
