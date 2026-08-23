// SPDX-License-Identifier: GPL-3.0-only

#include "load_store.hpp"

#include <xmipp4/core/platform/assert.hpp>
#include <core/numerical/float16_conversions.hpp>

namespace xmipp4
{
namespace cpu
{

template <typename T>
inline T load(const T *ptr) noexcept
{
	REX_ASSERT(ptr);
	return *ptr;
}

inline float load(const float16_t *ptr) noexcept
{
	REX_ASSERT(ptr);
	return float16_to_float(*ptr);
}

inline std::complex<float> load(const std::complex<float16_t> *ptr) noexcept
{
	REX_ASSERT(ptr);
	return std::complex<float>(
		float16_to_float(ptr->real()),
		float16_to_float(ptr->imag())
	);
}

template <typename T, typename U>
inline void store(T *ptr, const U &value) noexcept
{
	REX_ASSERT(ptr);
	*ptr = static_cast<T>(value);
}

inline void store(float16_t *ptr, float value) noexcept
{
	REX_ASSERT(ptr);
	*ptr = float_to_float16(value);
}

inline void store(
	std::complex<float16_t> *ptr,
	const std::complex<float> &value
) noexcept
{
	REX_ASSERT(ptr);
	*ptr = std::complex<float16_t>(
		float_to_float16(value.real()),
		float_to_float16(value.imag())
	);
}

template <typename T, typename Q>
inline void cast(T *destination, const Q *source) noexcept
{
	REX_ASSERT(destination);
	REX_ASSERT(source);
	*destination = static_cast<T>(*source);
}

template <typename Q>
inline typename std::enable_if<std::is_arithmetic<Q>::value>::type
cast(float16_t *destination, const Q *source) noexcept
{
	REX_ASSERT(source);
	store(destination, static_cast<float>(*source));
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type
cast(T *destination, const float16_t *source) noexcept
{
	REX_ASSERT(destination);
	*destination = static_cast<T>(load(source));
}

template <typename T, typename Q>
inline void cast(
	std::complex<T> *destination,
	const std::complex<Q> *source
) noexcept
{
	REX_ASSERT(destination);
	REX_ASSERT(source);
	const Q source_real = source->real();
	const Q source_imag = source->imag();
	T real;
	T imag;
	cast(&real, &source_real);
	cast(&imag, &source_imag);
	*destination = std::complex<T>(real, imag);
}

template <typename T, typename Q>
inline void cast(std::complex<T> *destination, const Q *source) noexcept
{
	REX_ASSERT(destination);
	T real;
	cast(&real, source);
	*destination = std::complex<T>(real);
}

} // namespace cpu
} // namespace xmipp4
