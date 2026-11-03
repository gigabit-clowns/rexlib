// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/core/plugin.hpp>
#include <rexlib/core/platform/dynamic_shared_object.h>

#if defined(REXLIB_DUMMY_PLUGIN2_EXPORTING)
    #define REXLIB_DUMMY_PLUGIN2_API REXLIB_EXPORT
#else
    #define REXLIB_DUMMY_PLUGIN2_API REXLIB_IMPORT
#endif

namespace rexlib
{

static const std::string name = "dummy-plugin2";

class dummy_plugin2 final
    : public rexlib::plugin
{
    const std::string& get_name() const noexcept final
    {
        return name;
    }

    version get_version() const noexcept final
    {
        return version(4, 5, 6);
    }

    void register_at(service_catalog&) const
    {
        // NO-OP
    }
};

} // namespace rexlib

static const rexlib::dummy_plugin2 instance;

extern "C"
{
REXLIB_DUMMY_PLUGIN2_API const rexlib::plugin* rexlib_get_plugin() 
{
    return &instance;
}
}
