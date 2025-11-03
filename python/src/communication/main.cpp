// SPDX-License-Identifier: GPL-3.0-only

#include "main.hpp"

#include "communicator_manager.hpp"
#include "communicator.hpp"

namespace xmipp4
{
namespace communication
{

namespace py = pybind11;

void bind_communication(pybind11::module_ &m)
{
	bind_communicator_manager(m);
	bind_communicator(m);
}

} // namespace communication
} // namespace xmipp4
