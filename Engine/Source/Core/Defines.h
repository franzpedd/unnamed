#pragma once

/// @brief DLL exporting
#if defined(_WIN32)
    #if defined(COSMOS_EXPORT)
        #define COSMOS_API __declspec(dllexport)
    #else
        #define COSMOS_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #if defined(COSMOS_EXPORT)
        #define COSMOS_API __attribute__((visibility("default")))
    #else
        #define COSMOS_API
    #endif
#else
    #define COSMOS_API
#endif

/// @brief quick macro to create a version
#define COSMOS_MAKE_VERSION(variant, major, minor, patch) ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))