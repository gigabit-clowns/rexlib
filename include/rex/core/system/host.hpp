// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

namespace rex 
{

/**
 * @brief Get the hostname of the machine.
 * 
 * @return std::string The hostname.
 */
std::string get_hostname();

/**
 * @brief Get the total memory available in the system.
 * 
 * @return std::size_t Total memory in bytes.
 */
std::size_t get_total_system_memory();

/**
 * @brief Get the memory available in the system.
 * 
 * @return std::size_t Total memory in bytes.
 */
std::size_t get_available_system_memory();

/**
 * @brief Get the page size of the memory management subsystem
 * 
 * @return std::size_t The page size.
 */
std::size_t get_page_size();

} // namespace rex
