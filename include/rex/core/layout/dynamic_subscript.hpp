// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <ostream>
#include <cstddef>

#include "subscript_tags.hpp"
#include "slice.hpp"
#include "index.hpp"
#include "../platform/attributes.hpp"
#include "../platform/cpp_explicit.hpp"

namespace rex 
{


/**
 * @brief Class describing a runtime defined subscript.
 * 
 * A subscript may be either a
 * - ellipsis_tag
 * - new_axis_tag
 * - index (std::ptrdiff_t)
 * - slice (slice)
 * 
 */
class dynamic_subscript
{
public:
	/**
	 * @brief Enumeration holding the types this object can hold. 
	 * 
	 */
	enum class subscript_type
	{
		ellipsis,
		new_axis,
		index,
		slice
	};

	/**
	 * @brief Construct a dynamic_subscript holding an ellipsis_tag.
	 * 
	 */
	REXLIB_NO_EXPLICIT REXLIB_CONSTEXPR
	dynamic_subscript(ellipsis_tag) noexcept;

	/**
	 * @brief Construct a dynamic_subscript holding a new_axis_tag.
	 * 
	 */
	REXLIB_NO_EXPLICIT REXLIB_CONSTEXPR
	dynamic_subscript(new_axis_tag) noexcept;

	/**
	 * @brief Construct a dynamic_subscript holding an index.
	 * 
	 * @param index The index that is assigned to this object.

		*/
	REXLIB_NO_EXPLICIT REXLIB_CONSTEXPR
	dynamic_subscript(std::ptrdiff_t index) noexcept;

	/**
	 * @brief Construct a dynamic_subscript holding a slice.
	 * 
	 * @param slice the slice that is assigned to this object.
	 * 
	 */
	REXLIB_NO_EXPLICIT REXLIB_CONSTEXPR
	dynamic_subscript(const slice &slice) noexcept;

	dynamic_subscript(const dynamic_subscript &other) = default;
	dynamic_subscript(dynamic_subscript &&other) = default;
	~dynamic_subscript() = default;

	dynamic_subscript& operator=(const dynamic_subscript &other) = default;
	dynamic_subscript& operator=(dynamic_subscript &&other) = default;

	/**
	 * @brief Get the subscript type held by this object.
	 * 
	 * @return subscript_type 
	 */
	REXLIB_CONSTEXPR
	subscript_type get_subscript_type() const noexcept;

	/**
	 * @brief Get the index.
	 * 
	 * This method shall only be called if get_subscript_type() returns
	 * subscript_type::index. Otherwise an exception is thrown.
	 * 
	 * @return std::ptrdiff_t The index held by this object.
	 * @throws bad_dynamic_subscript_access
	 */
	std::ptrdiff_t get_index() const;

	/**
	 * @brief Get the slice.
	 * 
	 * This method shall only be called if get_subscript_type() returns
	 * subscript_type::slice. Otherwise an exception is thrown.
	 * 
	 * @return slice The slice held by this object.
	 * @throws bad_dynamic_subscript_access
	 */
	slice get_slice() const;

	template <typename T>
	friend std::basic_ostream<T>& operator<<(
		std::basic_ostream<T>& os,
		const dynamic_subscript &subscript
	)
	{
		os << "dynamic_subscript(";
		visit(
			[&os] (auto x)
			{
				os << x;
			},
			subscript
		);
		return os << ")";
	}

private:
	using storage_type = std::array<std::ptrdiff_t, 3>;

	storage_type m_data; // When holding index, first value is used.
	subscript_type m_type;
};

class bad_dynamic_subscript_access
	: public std::logic_error
{
public:
	using std::logic_error::logic_error;
};

/**
 * @brief Invoke the provided function with the value held by the dynamic 
 * subscript
 * 
 * @tparam F Function type.
 * @param func The function to be invoked. Must be callable with the following 
 * argument types:
 * - ellipsis_tag
 * - new_axis_tag
 * - std::ptrdiff_t
 * - slice
 * 
 * @param subscript The subscript to be evaluated.
 * @return auto result of calling func with the evaluated argument.
 */
template <typename F>
REXLIB_CONSTEXPR
auto visit(F&& func, const dynamic_subscript &subscript);

} // namespace rex

#include "dynamic_subscript.inl"
