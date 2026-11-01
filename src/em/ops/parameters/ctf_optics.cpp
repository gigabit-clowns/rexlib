// SPDX-License-Identifier: GPL-3.0-only

#include <rexlib/em/ops/parameters/ctf_optics.hpp>

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rexlib
{
namespace em
{

namespace
{

double check_voltage(double voltage)
{
	if (!(std::isfinite(voltage) && voltage > 0.0))
	{
		std::ostringstream oss;
		oss << "ctf_optics: the voltage must be finite and positive, but it "
			<< "is " << voltage << ".";
		throw std::invalid_argument(oss.str());
	}
	return voltage;
}

double check_spherical_aberration(double spherical_aberration)
{
	if (!std::isfinite(spherical_aberration))
	{
		std::ostringstream oss;
		oss << "ctf_optics: the spherical aberration must be finite, but it "
			<< "is " << spherical_aberration << ".";
		throw std::invalid_argument(oss.str());
	}
	return spherical_aberration;
}

double check_amplitude_contrast(double amplitude_contrast)
{
	if (!(amplitude_contrast >= 0.0 && amplitude_contrast < 1.0))
	{
		std::ostringstream oss;
		oss << "ctf_optics: the amplitude contrast must lie in [0, 1), but "
			<< "it is " << amplitude_contrast << ".";
		throw std::invalid_argument(oss.str());
	}
	return amplitude_contrast;
}

} // anonymous namespace

ctf_optics::ctf_optics(
	double voltage,
	double spherical_aberration,
	double amplitude_contrast
)
	: m_voltage(check_voltage(voltage))
	, m_spherical_aberration(check_spherical_aberration(spherical_aberration))
	, m_amplitude_contrast(check_amplitude_contrast(amplitude_contrast))
{
}

double ctf_optics::get_voltage() const noexcept
{
	return m_voltage;
}

double ctf_optics::get_spherical_aberration() const noexcept
{
	return m_spherical_aberration;
}

double ctf_optics::get_amplitude_contrast() const noexcept
{
	return m_amplitude_contrast;
}

double ctf_optics::get_wavelength() const noexcept
{
	const auto volts = m_voltage * 1e3;
	return 12.2643247 / std::sqrt(volts * (1.0 + 0.978466e-6 * volts));
}

} // namespace em
} // namespace rexlib
