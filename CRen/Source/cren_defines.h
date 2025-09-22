#ifndef CREN_DEFINES_INCLUDED
#define CREN_DEFINES_INCLDUED

/// @brief align-as per compiler
#if defined(_MSC_VER)
    #define align_as(X) __declspec(align(X))
#elif defined(__GNUC__) || defined(__clang__)
    #define align_as(X) __attribute__((aligned(X)))
#else
    #define align_as(X) _Alignas(X)
#endif

/// @brief building as DLL or static-lib, depending on context
#ifdef CREN_SHARED_LIBRARY
	#ifdef PLATFORM_WINDOWS
		#ifdef CREN_BUILDING_DLL
			#define CREN_API __declspec(dllexport)
		#else
			#define CREN_API __declspec(dllimport)
		#endif
	#else
		#define CREN_API __attribute__((visibility("default")))
	#endif
#else
	#define CREN_API
#endif

/// @brief include standart bool and integer implementations
#include <stdbool.h>
#include <stdint.h>

#endif // CREN_DEFINES_INCLDUED