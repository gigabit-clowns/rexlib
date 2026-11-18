// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace rexlib
{
namespace cuda
{

/**
 * @brief Scoped selection of the calling thread's current CUDA device.
 *
 * Allocations, stream and event creation all belong to whichever device the
 * calling thread selected last, so every path reaching the runtime must
 * select its own device first and restore the previous selection on the way
 * out.
 */
class device_guard
{
public:
	explicit device_guard(int ordinal);
	device_guard(const device_guard &other) = delete;
	device_guard(device_guard &&other) = delete;
	~device_guard();

	device_guard& operator=(const device_guard &other) = delete;
	device_guard& operator=(device_guard &&other) = delete;

private:
	/// Spares the destructor a query when the device was already current.
	static constexpr int no_selection = -1;

	int m_previous;
};

} // namespace cuda
} // namespace rexlib
