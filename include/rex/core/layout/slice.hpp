// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/constexpr.hpp"
#include "../platform/attributes.hpp"

#include <cstddef>
#include <ostream>

namespace rex 
{

/**
 * @brief Class representing an slice of an array.
 *
 * This class is used to represent a slice of an array axis. It consists of
 * a start index, an element count and a step between elements.
 * 
 */
class slice
{
public:
	using start_type = std::ptrdiff_t;
	using count_type = std::size_t;
	using step_type = std::ptrdiff_t;

	/**
	 * @brief Constructor
	 * 
	 * @param start Index of the first element
	 * @param count Number of elements.
	 * @param step Step between consecutive elements
	 */
	REXLIB_CONSTEXPR 
	slice(start_type start, count_type count, step_type step ) noexcept;

	slice() = default;
	slice(const slice& other) = default;
	slice(slice&& other) = default;
	~slice() = default;
	
	slice& operator=(const slice& other) = default;
	slice& operator=(slice&& other) = default;

	/**
	 * @brief Set the starting index
	 * 
	 * @param start Index of the first element
	 */
	REXLIB_CONSTEXPR void set_start(start_type start) noexcept;

	/**
	 * @brief Get the starting index
	 * 
	 * @return start_type const& Index of the first element
	 */
	REXLIB_CONSTEXPR const start_type& get_start() const noexcept;
	
	/**
	 * @brief Set the index element count.
	 * 
	 * @param count New number of elements referred by this slice.
	 */
	REXLIB_CONSTEXPR void set_count(count_type count) noexcept;

	/**
	 * @brief Get the element count.
	 * 
	 * @return count_type const& Number of elements referred by this slice.
	 */
	REXLIB_CONSTEXPR const count_type& get_count() const noexcept;

	/**
	 * @brief Set the step
	 * 
	 * @param step Step between consecutive elements
	 */
	REXLIB_CONSTEXPR void set_step(step_type step) noexcept;

	/**
	 * @brief Get the step
	 * 
	 * @return step_type const& Step between consecutive elements
	 */
	REXLIB_CONSTEXPR const step_type& get_step() const noexcept;

	friend REXLIB_CONSTEXPR
	bool operator==(const slice &lhs, const slice &rhs) noexcept
	{
		return
			lhs.get_start() == rhs.get_start() &&
			lhs.get_count() == rhs.get_count() &&
			lhs.get_step() == rhs.get_step();
	}

	friend REXLIB_CONSTEXPR
	bool operator!=(const slice &lhs, const slice &rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template <typename T>
	friend std::basic_ostream<T>&
	operator<<(std::basic_ostream<T> &os, const slice &s)
	{
		return
			os << "slice(" << s.get_start()
			<< ", " << s.get_count()
			<< ", " << s.get_step() << ")";
	}

private:
	start_type m_start;
	count_type m_count;
	step_type m_step;
};

/**
 * @brief Get the value representing the end of an array to be used as
 * count.
 * 
 * @return std::size_t end value
 * 
 */
REXLIB_CONSTEXPR std::size_t end() noexcept;

/**
 * @brief Construct an slice containing all positions in the axis.
 * 
 * @return An slice representing all positions in the axis.
 */
REXLIB_CONSTEXPR slice all() noexcept;

/**
 * @brief Construct an slice containing even positions in the axis.
 * 
 * @return An slice representing even positions in the axis.
 */
REXLIB_CONSTEXPR slice even() noexcept;

/**
 * @brief Construct an slice containing odd positions in the axis.
 * 
 * @return An slice representing odd positions in the axis.
 */
REXLIB_CONSTEXPR slice odd() noexcept;

/**
 * @brief Create an slice using its count value.
 * 
 * The start is defaulter to be 0.
 * The step is defaulted to be 1.
 * 
 * @param count Number of elements.
 * @return slice An slice representing the first count positions in the axis.
 */
REXLIB_CONSTEXPR 
slice make_slice(std::size_t count) noexcept;

/**
 * @brief Create an slice using its start and count values.
 * 
 * The step is defaulted to be 1.
 * 
 * @param start Index of the first element
 * @param count Number of elements.
 * @return slice An slice representing the first count positions starting from
 * start.
 */
REXLIB_CONSTEXPR 
slice make_slice(std::ptrdiff_t start, std::size_t count) noexcept;

/**
 * @brief Create an slice using its start and count values.
 * 
 * The step is defaulted to be 1.
 * 
 * @param start Index of the first element
 * @param count Number of elements.
 * @param step Jump between consecutive elements.
 * @return slice An slice representing the first count positions starting from
 * start from step to step.
 */
REXLIB_CONSTEXPR 
slice make_slice(
	std::ptrdiff_t start, 
	std::size_t count, 
	std::ptrdiff_t step
) noexcept;

/**
 * @brief Sanitize a slice to ensure it is valid and within bounds for a given 
 * extent.
 *
 * When end() is provided as count, it computes and replaces it by the actual 
 * element count.
 * 
 * @param slice Slice to be sanitized.
 * @param extent Extent of the axis to which the slice applies.
 * @return slice Sanitized slice.
 * @throw std::out_of_range If the slice is invalid or out of bounds.
 * @throw std::invalid_argument If the slice step is zero.
 */
slice sanitize_slice(const slice &slice, std::size_t extent);

} // namespace rex

#include "slice.inl"
