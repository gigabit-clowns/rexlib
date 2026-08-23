// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "../platform/attributes.hpp"
#include "../platform/constexpr.hpp"

#include <initializer_list>
#include <type_traits>
#include <functional>

namespace xmipp4
{

/**
 * @brief Stores a set of bits in the form of flags
 * Similar to std::bitset but instead of a index based
 * interface it provides a enum based interface.
 * 
 * @tparam B enum type which contains the declaration of
 * the flag bits
 */
template<typename B>
class flagset {
public:
	/**
	 * @brief The provided enum declaration of the flag bits
	 * 
	 */
	using bit_type = B;

	/**
	 * @brief The underlying type of the provided enum
	 * 
	 */
	using underlying_type = typename std::underlying_type<bit_type>::type;
	
	/**
	 * @brief Unsigned underlying type of the provided enum
	 * 
	 */
	using unsigned_type = typename std::make_unsigned<underlying_type>::type;

	/**
	 * @brief Empty constructor. It initializes the set as none
	 * of the flags were set
	 * 
	 */
	REX_CONSTEXPR flagset() noexcept;

	/**
	 * @brief Initializes the set with a single bit set
	 * 
	 * @param bit The bit to be set
	 *
	 */
	REX_CONSTEXPR flagset(bit_type bit) noexcept;

	/**
	 * @brief Constructs the flagset from raw binary data
	 * 
	 * @param data The binary data
	 */
	explicit REX_CONSTEXPR flagset(underlying_type data) noexcept;

	/**
	 * @brief Initializes the set with the elements inside the
	 * a iterator range
	 * 
	 * @tparam It forward iterator
	 * @param first Iterator to the first element
	 * @param last Iterator to the past the end element
	 * 
	 */
	template<typename It>
	REX_CONSTEXPR flagset(It first, It last) noexcept;

	/**
	 * @brief Initializes the set with a initializer list
	 * 
	 * @param bits Initializer list with the flags to be set
	 * 
	 */
	REX_CONSTEXPR flagset(std::initializer_list<bit_type> bits) noexcept;
	
	/**
	 * @brief Copy constructor
	 * 
	 * @param other The set to be copied from
	 */
	flagset(const flagset &other) = default;

	/**
	 * @brief Move constructor. Same behaviour as copy constructor
	 * 
	 * @param other The set to be copied from
	 */
	flagset(flagset &&other) noexcept = default;

	/**
	 * @brief Destroy the flagset object
	 * 
	 */
	~flagset() = default;

	/**
	 * @brief Copy assign operator
	 * 
	 * @param other The set to be copied from
	 * @return flagset& *this
	 */
	flagset& operator=(const flagset &other) = default;
	
	/**
	 * @brief Move assign operator. Same behaviour as copy constructor
	 * 
	 * @param other The set to be copied from
	 * @return flagset& *this
	 */
	flagset& operator=(flagset &&other) noexcept = default;

	/**
	 * @brief Checks if the set is nonzero
	 * 
	 * @return true If there is one or more bits set
	 * @return false If there are no bits set
	 */
	REX_CONSTEXPR operator bool() const noexcept;

	/**
	 * @brief Converts to the underlying type of the bit definition
	 * bits set
	 * 
	 * @return underlying_type
	 */
	explicit REX_CONSTEXPR operator underlying_type() const noexcept;

	/**
	 * @brief In-place union with another flagset
	 * 
	 * @param rhs The other flagset
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& operator|=(const flagset& rhs) noexcept;

	/**
	 * @brief In-place intersection with another flagset
	 * 
	 * @param rhs The other flagset
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& operator&=(const flagset& rhs) noexcept;
	
	/**
	 * @brief In-place XOR with another flagset
	 * 
	 * @param rhs The other flagset
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& operator^=(const flagset& rhs) noexcept;

	/**
	 * @brief Compares for equality with another flagset
	 * 
	 * @param lhs One flagset
	 * @param rhs The other flagset
	 * @return true If both flagset-s are equal
	 * @return false If both flagset-s are not equal
	 */
	friend REX_CONSTEXPR
	bool operator==(const flagset& lhs, const flagset& rhs) noexcept
	{
		return lhs.m_data == rhs.m_data;
	}

	/**
	 * @brief Compares for inequality with another flagset
	 * 
	 * @param lhs One flagset
	 * @param rhs The other flagset
	 * @return true If both flagset-s are not equal
	 * @return false If both flagset-s are equal
	 */
	friend REX_CONSTEXPR
	bool operator!=(const flagset& lhs, const flagset& rhs) noexcept
	{
		return lhs.m_data != rhs.m_data;
	}

	/**
	 * @brief Obtain the underlying integer representation of the flags.
	 * 
	 * @return underlying_type Unsigned integer with the data.
	 */
	REX_CONSTEXPR underlying_type get_bits() const noexcept;

	/**
	 * @brief Tests if the requested flag is set.
	 * 
	 * @param bit The flag to be tested.
	 * @return true If the bit is set.
	 * @return false If the bit is not set.
	 */
	REX_CONSTEXPR bool contains(const bit_type& bit) const noexcept;

	/**
	 * @brief Checks if all of a set of flags are present in this
	 * 
	 * @param other The set of flags that needs to be present here
	 * @return true If all the flags are present
	 * @return false If all the flags are not present
	 */
	REX_CONSTEXPR bool all_of(const flagset& other) const noexcept;
	
	/**
	 * @brief Checks if any of a set of flags are present in this
	 * 
	 * @param other The set of flags that needs to be present here
	 * @return true If any the flags are present
	 * @return false If any the flags are not present
	 */
	REX_CONSTEXPR bool any_of(const flagset& other) const noexcept;
	
	/**
	 * @brief Checks if none of a set of flags are present in this
	 * 
	 * @param other The set of flags that needs to be absent here
	 * @return true If none the flags are present
	 * @return false If any the flags are is present
	 */
	REX_CONSTEXPR bool none_of(const flagset& other) const noexcept;
	
	/**
	 * @brief Checks if only a set of flags are present in this
	 * 
	 * @param other The set of allowed flags
	 * @return true If only the allowed flags are present
	 * @return false If any of the disallowed flags is present
	 */
	REX_CONSTEXPR bool only_of(const flagset& other) const noexcept;

	/**
	 * @brief Counts the number of flags that are present
	 * 
	 * @return int The number of flags present here
	 */
	REX_CONSTEXPR int count() const noexcept;

	/**
	 * @brief Computes the parity of the stored flags
	 * 
	 * @return true when the number of flags is odd
	 * @return false when the number of flags is even
	 */
	REX_CONSTEXPR bool parity() const noexcept;

	/**
	 * @brief Checks if only one flag is present
	 * 
	 * @return true if only one flag is present
	 * @return false if zero or more than one flags are present
	 */
	REX_CONSTEXPR bool has_single_bit() const noexcept;

	/**
	 * @brief Set a particular set of flags to the desired value
	 * 
	 * @param other The set of flags to be changed
	 * @param value The value of the flags
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& set(const flagset& other, bool value) noexcept;

	/**
	 * @brief Sets a particular set of flags
	 * 
	 * @param other The set of flags to be set
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& set(const flagset& other) noexcept;

	/**
	 * @brief Clears all flags
	 * 
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& clear() noexcept;
	
	/**
	 * @brief Clears a particular set of flags
	 * 
	 * @param other The set of flags to be cleared
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& clear(const flagset& other) noexcept;
	
	/**
	 * @brief Toggles a particular set of flags
	 * 
	 * @param other The set of flags to be toggled
	 * @return flagset& *this 
	 */
	REX_CONSTEXPR flagset& toggle(const flagset& other) noexcept;

	friend REX_CONSTEXPR
	flagset operator|(const flagset& lhs, const flagset& rhs) noexcept
	{
		return flagset(lhs.m_data | rhs.m_data);
	}

	friend REX_CONSTEXPR
	flagset operator&(const flagset& lhs, const flagset& rhs) noexcept
	{
		return flagset(lhs.m_data & rhs.m_data);
	}

	friend REX_CONSTEXPR
	flagset operator^(const flagset& lhs, const flagset& rhs) noexcept
	{
		return flagset(lhs.m_data ^ rhs.m_data);
	}

private:
	unsigned_type m_data;
};

} // namespace xmipp4

template <typename B>
struct std::hash<xmipp4::flagset<B>>
{
	REX_CONSTEXPR size_t operator()(xmipp4::flagset<B> b) const noexcept;
};

#include "flagset.inl"
