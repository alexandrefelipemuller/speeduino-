#pragma once

/** @file 
 * This file contains various shared utility macros that make writing function
 * style macros easier.
*/

#if !defined(UNUSED)
/** @brief Used to suppress unused parameter compiler warnings */
#define UNUSED(x) \
    (void)(x)
#endif

// Native GCC provides _countof(), but it doesn't work on an array *within*
// a packed struct
#if defined(NATIVE_BOARD)
#undef _countof
#endif 

/** @brief Compile time calculation of an array size */
#if !defined(_countof)
#define _countof(x) \
    (sizeof((x)) / sizeof ((x)[0]))
#endif

/** @brief Obtain a pointer to 1 *element* past the end of an array */
#if !defined(_end_range_address)
#define _end_range_address(array) \
    ((array) + _countof((array)))
#endif

/** @brief Obtain a pointer to 1 *byte* past the end of an array */
#if !defined(_end_range_byte_address)
#define _end_range_byte_address(array) \
    (((byte*)(array)) + sizeof((array)))
#endif

/** @brief Pre-processor arithmetic increment (pulled from Boost.Preprocessor) */
#if !defined(PP_INC)
#define PP_INC(x) \
    PP_INC_I(x)
#endif

/// @cond 
// PP_INC() support macros
#define PP_INC_I(x) PP_INC_ ## x
#define PP_INC_0 1 // NOSONAR
#define PP_INC_1 2 // NOSONAR
#define PP_INC_2 3 // NOSONAR
#define PP_INC_3 4 // NOSONAR
#define PP_INC_4 5 // NOSONAR
#define PP_INC_5 6 // NOSONAR
#define PP_INC_6 7 // NOSONAR
#define PP_INC_7 8 // NOSONAR
#define PP_INC_8 9 // NOSONAR
#define PP_INC_9 10 // NOSONAR
#define PP_INC_10 11 // NOSONAR
#define PP_INC_11 12 // NOSONAR
#define PP_INC_12 13 // NOSONAR
/// @endcond

/// @cond 
// CONCAT() support macros
#define CAT_HELPER(a, b) a ## b
/// @endcond

/** @brief Concatenate A & B *after* macro expansion */
#if !defined(CONCAT)
#define CONCAT(A, B) \
    CAT_HELPER(A, B)
#endif

/** @brief Force an out-of-line function (I.e. defined in a cpp file) to be inlined. */
#define BEGIN_LTO_ALWAYS_INLINE(returnType) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wattributes\"") \
    returnType __attribute__((always_inline)) // cppcheck-suppress misra-c2012-20.7
#define END_LTO_INLINE() \
    _Pragma("GCC diagnostic pop")

// Optional module feature flags.
// Phase 1 keeps current behaviour by enabling all modules by default.
#ifndef FEATURE_MODULE_LOGGING
#define FEATURE_MODULE_LOGGING 1
#endif

#ifndef FEATURE_MODULE_SECONDARY_SERIAL
#define FEATURE_MODULE_SECONDARY_SERIAL 1
#endif

#ifndef FEATURE_MODULE_COMMS_EXTENDED
#define FEATURE_MODULE_COMMS_EXTENDED 1
#endif

#ifndef FEATURE_MODULE_BOOST
#define FEATURE_MODULE_BOOST 1
#endif

#ifndef FEATURE_MODULE_VVT
#define FEATURE_MODULE_VVT 1
#endif

#ifndef FEATURE_MODULE_ENGINE_PROTECTION
#define FEATURE_MODULE_ENGINE_PROTECTION 1
#endif

#ifndef FEATURE_MODULE_LAUNCH_FLATSHIFT
#define FEATURE_MODULE_LAUNCH_FLATSHIFT 1
#endif

#ifndef FEATURE_MODULE_FAN_AIRCON
#define FEATURE_MODULE_FAN_AIRCON 1
#endif

#ifndef FEATURE_MODULE_PROGRAMMABLE_IO
#define FEATURE_MODULE_PROGRAMMABLE_IO 1
#endif

#ifndef FEATURE_MODULE_WMI
#define FEATURE_MODULE_WMI 1
#endif

#ifndef FEATURE_MODULE_TABLE_SWITCHING
#define FEATURE_MODULE_TABLE_SWITCHING 1
#endif

#ifndef FEATURE_MODULE_ADVANCED_ENGINE
#define FEATURE_MODULE_ADVANCED_ENGINE 1
#endif
