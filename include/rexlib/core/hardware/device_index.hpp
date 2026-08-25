// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../platform/dynamic_shared_object.h"

#include <string>
#include <string_view>
#include <ostream>

namespace rexlib 
{

/**
 * @brief Location of a device as a (backend, id) tuple.
 *
 * A @ref device_index is a unique identifier within a @ref device_manager of a
 * physical device. 
 * 
 * Default-constructed indices hold an empty backend name and a zero
 * device id; such an index does not refer to any real device and is
 * intended only as a placeholder before assignment or parsing.
 */
class device_index
{
public:
	/**
	 * @brief Construct a new device index from its components.
	 *
	 * @param backend_name Name of the backend. Must match the name
	 * reported by the target @ref device_backend for the index to refer
	 * to a real device.
	 * @param device_id ID of the device within @p backend_name, as
	 * returned by @ref device_backend::enumerate_devices.
	 */
	REXLIB_API
	device_index(const std::string &backend_name, std::size_t device_id);

	/**
	 * @brief Construct a new device index from its components.
	 *
	 * Move overload that avoids copying @p backend_name.
	 *
	 * @param backend_name Name of the backend (moved from).
	 * @param device_id ID of the device within @p backend_name, as
	 * returned by @ref device_backend::enumerate_devices.
	 */
	REXLIB_API
	device_index(std::string &&backend_name, std::size_t device_id);

	REXLIB_API
	device_index() noexcept;
	REXLIB_API
	device_index(const device_index &other);
	REXLIB_API
	device_index(device_index &&other) noexcept;
	REXLIB_API
	~device_index();

	REXLIB_API
	device_index& operator=(const device_index &other);
	REXLIB_API
	device_index& operator=(device_index &&other) noexcept;

	/**
	 * @brief Get the name of the device backend.
	 *
	 * @return const std::string& Reference to the stored backend name.
	 * The reference is valid for the lifetime of this @ref device_index
	 * and is invalidated by assignment to or destruction of the object.
	 */
	REXLIB_API
	const std::string& get_backend_name() const noexcept;

	/**
	 * @brief Get the ID of the device within the backend.
	 *
	 * @return std::size_t The backend-local device ID. The value is only
	 * meaningful when interpreted by the backend identified by
	 * @ref get_backend_name.
	 */
	REXLIB_API
	std::size_t get_device_id() const noexcept;

	friend bool
	operator==(const device_index &lhs, const device_index &rhs) noexcept
	{
		return lhs.get_device_id() == rhs.get_device_id() &&
			lhs.get_backend_name() == rhs.get_backend_name();
	}

	friend bool
	operator!=(const device_index &lhs, const device_index &rhs) noexcept
	{
		return lhs.get_device_id() != rhs.get_device_id() ||
			lhs.get_backend_name() != rhs.get_backend_name();
	}

	friend bool
	operator<(const device_index &lhs, const device_index &rhs) noexcept
	{
		if (lhs.get_backend_name() < rhs.get_backend_name())
		{
			return true;
		}
		else if(lhs.get_backend_name() == rhs.get_backend_name())
		{
			return lhs.get_device_id() < rhs.get_device_id();
		}
		return false;
	}

	friend bool
	operator<=(const device_index &lhs, const device_index &rhs) noexcept
	{
		if (lhs.get_backend_name() < rhs.get_backend_name())
		{
			return true;
		}
		else if(lhs.get_backend_name() == rhs.get_backend_name())
		{
			return lhs.get_device_id() <= rhs.get_device_id();
		}
		return false;
	}

	friend bool
	operator>(const device_index &lhs, const device_index &rhs) noexcept
	{
		return rhs < lhs;
	}

	friend bool
	operator>=(const device_index &lhs, const device_index &rhs) noexcept
	{
		return rhs <= lhs;
	}

	friend std::ostream&
	operator<<(std::ostream &os, const device_index &index)
	{
		return os << index.get_backend_name() << ':' << index.get_device_id();
	}

private:
	std::string m_backend_name;
	std::size_t m_device_id;
};

/**
 * @brief Parse the device ID from a string.
 *
 * A device ID string representation is expected to be:
 * <device_backend>:<device_id>
 * <device_backend> (assumes device_id=0)
 *
 * @param text Appropriately formatted string with the device ID
 * @param result Output device_index object.
 * @return true The string was parsed successfully and the result was written.
 * @return false The string was not parsed and the result was not written.
 */
REXLIB_API
bool parse_device_index(std::string_view text, device_index &result);

} // namespace rexlib
