// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "c_attributes.h"
#include "operating_system.h"

/**
 * @def XMIPP4_IMPORT
 * @brief Declares that the function should be imported from a shared object
 * 
 */
#if defined(XMIPP4_WINDOWS)
	#if XMIPP4_HAS_GCC_ATTRIBUTE(dllimport)
		#define XMIPP4_IMPORT XMIPP4_GCC_ATTRIBUTE(dllimport)
	#else
		#define XMIPP4_IMPORT __declspec(dllimport)
	#endif
#elif XMIPP4_HAS_GCC_ATTRIBUTE(visibility)
	#define XMIPP4_IMPORT XMIPP4_GCC_ATTRIBUTE(visibility("default"))
#else
	#define XMIPP4_IMPORT
#endif

/**
 * @def XMIPP4_EXPORT
 * @brief Declares that the function should be exported to a shared object
 * 
 */
#if defined(XMIPP4_WINDOWS)
	#if XMIPP4_HAS_GCC_ATTRIBUTE(dllexport)
		#define XMIPP4_EXPORT XMIPP4_GCC_ATTRIBUTE(dllexport)
	#else
		#define XMIPP4_EXPORT __declspec(dllexport)
	#endif
#elif XMIPP4_HAS_GCC_ATTRIBUTE(visibility)
	#define XMIPP4_EXPORT XMIPP4_GCC_ATTRIBUTE(visibility("default"))
#else
	#define XMIPP4_EXPORT
#endif

/**
 * @def XMIPP4_LOCAL
 * @brief Declares that the function is only used used locally at the current 
 * shared object
 * 
 */
#if defined(XMIPP4_WINDOWS)
	#define XMIPP4_LOCAL
#elif XMIPP4_HAS_GCC_ATTRIBUTE(visibility)
	#define XMIPP4_LOCAL XMIPP4_GCC_ATTRIBUTE(visibility("hidden"))
#else
	#define XMIPP4_LOCAL
#endif

/**
 * @def REXLIB_API
 * @brief Declares that the function is part of the public core API of XMIPP4
 * 
 * The functions declared as public core API will be exported to the shared object. 
 * 
 */
#if defined(REXLIB_NO_EXPORTS)
	#define REXLIB_API
#else
	#if defined(REXLIB_EXPORTING)
		#define REXLIB_API XMIPP4_EXPORT
	#else
		#define REXLIB_API XMIPP4_IMPORT
	#endif
#endif

/**
 * @def XMIPP4_STD_BASE_INTERFACE
 * @brief Silence MSVC warning C4275 for the class declared right after it.
 *
 * C4275 fires when a dll-interface class (see @ref REXLIB_API) derives
 * from a standard library type (e.g. std::runtime_error) that is not itself a
 * dll-interface. This is safe as long as every module links against the same
 * dynamic C++ runtime (/MD), so the base class has a single shared definition.
 *
 * Placed on its own line immediately before the class declaration, it
 * suppresses the warning for that single line only (via warning(suppress)), so
 * it needs no closing macro. The class head and its base-clause must therefore
 * be kept together on that one line. Expands to nothing on other compilers.
 *
 */
#if defined(_MSC_VER)
	#define XMIPP4_STD_BASE_INTERFACE __pragma(warning(suppress: 4275))
#else
	#define XMIPP4_STD_BASE_INTERFACE
#endif

/**
 * @def XMIPP4_STD_MEMBER_INTERFACE
 * @brief Silence MSVC warning C4251 for the data member declared right after it.
 *
 * C4251 fires when a dll-interface class (see @ref REXLIB_API) has a data
 * member whose type is not itself a dll-interface, typically a standard library
 * type such as the std::unique_ptr used for the pimpl idiom or an std::string.
 * This is safe as long as every module links against the same dynamic C++
 * runtime (/MD), so the type has a single shared definition, and the member is
 * not manipulated across the DLL boundary (private members never are).
 *
 * Placed on its own line immediately before a member declaration, it suppresses
 * the warning for that single line only (via warning(suppress)), so it needs no
 * closing macro and leaves the warning active everywhere else as a tripwire for
 * genuinely public non-dll-interface members. Expands to nothing on other
 * compilers.
 *
 */
#if defined(_MSC_VER)
	#define XMIPP4_STD_MEMBER_INTERFACE __pragma(warning(suppress: 4251))
#else
	#define XMIPP4_STD_MEMBER_INTERFACE
#endif

/**
 * @def XMIPP4_BEGIN_TEMPLATE_BASE
 * @brief Silence MSVC warning C4275 for a class deriving from a template.
 *
 * C4275 fires when a dll-interface class (see @ref REXLIB_API) derives
 * from a class template specialization, which is never itself a
 * dll-interface. This is safe as long as the template is header only, so
 * every module instantiates the same definition.
 *
 * Unlike @ref XMIPP4_STD_BASE_INTERFACE this comes as a pair, because the
 * class head and its base-clause cannot be kept on a single line while
 * respecting the line length limit. Close it with
 * @ref XMIPP4_END_TEMPLATE_BASE right after the class definition. Expands
 * to nothing on other compilers.
 *
 */
#if defined(_MSC_VER)
	#define XMIPP4_BEGIN_TEMPLATE_BASE \
		__pragma(warning(push)) \
		__pragma(warning(disable: 4275))
#else
	#define XMIPP4_BEGIN_TEMPLATE_BASE
#endif

/**
 * @def XMIPP4_END_TEMPLATE_BASE
 * @brief Close a @ref XMIPP4_BEGIN_TEMPLATE_BASE region.
 *
 */
#if defined(_MSC_VER)
	#define XMIPP4_END_TEMPLATE_BASE __pragma(warning(pop))
#else
	#define XMIPP4_END_TEMPLATE_BASE
#endif
