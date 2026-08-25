// SPDX-License-Identifier: GPL-3.0-only

#include "numerical_type_domain.hpp"

namespace rexlib
{

namespace detail
{

static_assert(
	static_cast<int>(numerical_type::count) <=
	static_cast<int>(sizeof(numerical_type_domain::mask_type) * 8),
	"numerical_type no longer fits in a numerical_type_domain mask. Widen "
	"numerical_type_domain::mask_type."
);

/**
 * @brief Get the bit representing a type within a domain mask.
 *
 * @param type The type. Must be representable.
 * @return numerical_type_domain::mask_type The bit.
 */
REXLIB_INLINE_CONSTEXPR
numerical_type_domain::mask_type
numerical_type_domain_bit(numerical_type type) noexcept
{
	return numerical_type_domain::mask_type(1) << static_cast<int>(type);
}

/**
 * @brief Get the mask holding every representable type.
 *
 * @return numerical_type_domain::mask_type The mask.
 */
REXLIB_INLINE_CONSTEXPR
numerical_type_domain::mask_type
numerical_type_domain_all_bits() noexcept
{
	return (
		numerical_type_domain::mask_type(1) <<
		static_cast<int>(numerical_type::count)
	) - numerical_type_domain::mask_type(1);
}

} // namespace detail

REXLIB_INLINE_CONSTEXPR
bool is_domain_representable(numerical_type type) noexcept
{
	return static_cast<int>(type) >= 0 &&
	       static_cast<int>(type) < static_cast<int>(numerical_type::count);
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain::numerical_type_domain() noexcept
	: m_mask(0)
{
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain::numerical_type_domain(mask_type mask) noexcept
	: m_mask(mask)
{
}

REXLIB_INLINE_CONSTEXPR_CPP14
numerical_type_domain::numerical_type_domain(
	std::initializer_list<numerical_type> types
) noexcept
	: m_mask(0)
{
	for (const auto type : types)
	{
		if (is_domain_representable(type))
		{
			m_mask |= detail::numerical_type_domain_bit(type);
		}
	}
}

REXLIB_INLINE_CONSTEXPR
bool numerical_type_domain::contains(numerical_type type) const noexcept
{
	return is_domain_representable(type) &&
	       (m_mask & detail::numerical_type_domain_bit(type)) != 0;
}

REXLIB_INLINE_CONSTEXPR
bool numerical_type_domain::empty() const noexcept
{
	return m_mask == 0;
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain::mask_type
numerical_type_domain::get_mask() const noexcept
{
	return m_mask;
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain numerical_type_domain::all() noexcept
{
	return numerical_type_domain(detail::numerical_type_domain_all_bits());
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain numerical_type_domain::none() noexcept
{
	return numerical_type_domain();
}

REXLIB_INLINE_CONSTEXPR
numerical_type_domain operator~(const numerical_type_domain &domain) noexcept
{
	return numerical_type_domain(
		~domain.get_mask() & detail::numerical_type_domain_all_bits()
	);
}



REXLIB_INLINE_CONSTEXPR_CPP14
numerical_type_domain make_numerical_type_domain(
	numerical_type_category category
) noexcept
{
	numerical_type_domain::mask_type mask = 0;

	const auto count = static_cast<int>(numerical_type::count);
	for (int i = 0; i < count; ++i)
	{
		const auto type = static_cast<numerical_type>(i);
		if (get_category(type) == category)
		{
			mask |= detail::numerical_type_domain_bit(type);
		}
	}

	return numerical_type_domain(mask);
}

} // namespace rexlib
