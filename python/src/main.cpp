
#include "communication/main.hpp"
#include "compute/main.hpp"
#include "image/main.hpp"

#include "interface_registry.hpp"
#include "plugin_manager.hpp"
#include "plugin.hpp"
#include "version.hpp"

#include <pybind11/pybind11.h>

#include <xmipp4/core/core_version.hpp>

#include <sstream>

using namespace xmipp4;

static std::string version_to_string(version ver)
{
    std::ostringstream oss;
    oss << ver;
    return oss.str();
}

PYBIND11_MODULE(_core_binding, m) {
    m.attr("__version__") = version_to_string(get_core_version());

    auto communication_module = m.def_submodule("communication");
    communication::bind_communication(communication_module);
    auto compute_module = m.def_submodule("compute");
    compute::bind_compute(compute_module);
    auto image_module = m.def_submodule("image");
    image::bind_image(image_module);

    bind_interface_registry(m);
    bind_plugin_manager(m);
    bind_plugin(m);
    bind_version(m);
}
