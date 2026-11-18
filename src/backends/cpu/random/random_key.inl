// SPDX-License-Identifier: GPL-3.0-only

#include "random_key.hpp"

#include "philox.hpp"

namespace rexlib
{
namespace cpu
{

inline
std::uint64_t derive_key(std::uint64_t key, std::uint64_t data) noexcept
{
	return make_philox_halves(
		philox4x32_10(
			make_philox_counter(data, 0, philox_domain::key_derivation),
			make_philox_key(key)
		)
	)[0];
}

} // namespace cpu
} // namespace rexlib
