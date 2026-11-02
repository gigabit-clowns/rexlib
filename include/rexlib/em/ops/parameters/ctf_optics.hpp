// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <rexlib/core/platform/dynamic_shared_object.h>

namespace rexlib
{
namespace em
{

/**
 * @brief Constants of a microscope that the contrast transfer function
 * depends on.
 */
class ctf_optics
{
public:
	/**
	 * @brief Construct the optics of a microscope.
	 *
	 * @param voltage The acceleration voltage, in kilovolts.
	 * @param spherical_aberration The spherical aberration coefficient, in
	 * millimetres.
	 * @param amplitude_contrast The fraction of amplitude contrast.
	 *
	 * @throws std::invalid_argument When the voltage is not finite and
	 * positive, the spherical aberration is not finite, or the amplitude
	 * contrast lies outside [0, 1).
	 */
	REXLIB_API
	ctf_optics(
		double voltage,
		double spherical_aberration,
		double amplitude_contrast
	);

	/**
	 * @brief Get the acceleration voltage.
	 *
	 * @return double The voltage, in kilovolts.
	 */
	REXLIB_API
	double get_voltage() const noexcept;

	/**
	 * @brief Get the spherical aberration coefficient.
	 *
	 * @return double The coefficient, in millimetres.
	 */
	REXLIB_API
	double get_spherical_aberration() const noexcept;

	/**
	 * @brief Get the fraction of amplitude contrast.
	 *
	 * @return double The fraction.
	 */
	REXLIB_API
	double get_amplitude_contrast() const noexcept;

	/**
	 * @brief Get the relativistic wavelength of the electrons.
	 *
	 * The wavelength is 12.2643247 / sqrt(V (1 + 0.978466e-6 V)), with V
	 * the voltage in volts.
	 *
	 * @return double The wavelength, in angstroms.
	 */
	REXLIB_API
	double get_wavelength() const noexcept;

private:
	double m_voltage;
	double m_spherical_aberration;
	double m_amplitude_contrast;
};

} // namespace em
} // namespace rexlib
