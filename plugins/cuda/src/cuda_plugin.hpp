#pragma once

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 * @file cuda_plugin.hpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Definition of the cuda_plugin class
 * @date 2024-10-26
 * 
 */


#include <xmipp4/core/plugin.hpp>

namespace xmipp4 
{

class cuda_plugin final
    : public plugin
{
public:
    cuda_plugin() = default;
    cuda_plugin(const cuda_plugin& other) = default;
    cuda_plugin(cuda_plugin&& other) = default;
    ~cuda_plugin() override = default;

    cuda_plugin& operator=(const cuda_plugin& other) = default;
    cuda_plugin& operator=(cuda_plugin&& other) = default;

    const std::string& get_name() const noexcept override;
    version get_version() const noexcept override;
    void register_at(interface_catalog& catalog) const override;

private:
    static const std::string name;

};

} // namespace xmipp4
