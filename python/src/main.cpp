
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

PYBIND11_MODULE(xmipp4, m) {
    m.attr("__version__") = version_to_string(get_core_version());

    register_version(m);
}
