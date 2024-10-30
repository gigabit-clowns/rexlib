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
 * @file cuda_plugin_hook.cpp
 * @author Oier Lauzirika Zarrabeitia (oierlauzi@bizkaia.eu)
 * @brief Exports the entry point for this plugin.
 * @date 2024-10-30
 * 
 */

#include "cuda_plugin.hpp"

#include <xmipp4/core/platform/dynamic_shared_object.h>

#if defined(XMIPP4_COMPUTE_CUDA_EXPORTING)
    #define XMIPP4_COMPUTE_CUDA_API XMIPP4_EXPORT
#else
    #define XMIPP4_COMPUTE_CUDA_API XMIPP4_IMPORT
#endif

static const xmipp4::cuda_plugin instance;

extern "C"
{
XMIPP4_COMPUTE_CUDA_API const xmipp4::plugin* xmipp4_get_plugin() 
{
    return &instance;
}
}
