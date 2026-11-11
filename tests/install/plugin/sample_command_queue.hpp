// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/hardware/command_queue.hpp>

namespace sample
{

/**
 * @brief A queue that runs each command as it is submitted, on the calling
 * thread.
 *
 * It runs the programs of the sample backend only, and throws
 * std::invalid_argument for any other.
 */
class sample_command_queue final
	: public rexlib::command_queue
{
public:
	rexlib::command_token submit(rexlib::command cmd) override;
};

} // namespace sample
