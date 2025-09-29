#ifndef CREN_PLATFORM_INCLUDED
#define CREN_PLATFORM_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief what underneath platform is being used
typedef enum CRen_Platform
{
	CREN_PLATFORM_UNKNOWN = -2,
	CREN_PLATFORM_UNKNOWN_APPLE = -1,
	CREN_PLATFORM_MACOS,
	CREN_PLATFORM_IOS,
	CREN_PLATFORM_X11,
	CREN_PLATFORM_WAYLAND,
	CREN_PLATFORM_ANDROID,
	CREN_PLATFORM_WINDOWS
} CRen_Platform;

/// @brief attempts to detect what windowing/operating system is currently being used
CREN_API CRen_Platform cren_detect_platform();

/// @brief creates a window surface for the underneath window (optionalHandle: wayland's surface OR x11 display)
CREN_API bool cren_surface_create(void* instance, void* surface, void* nativeWindow, void* optionalHandle);

/// @brief formats a const char* with a disk address of a file, constructing it's path
CREN_API void cren_get_path(const char* subpath, const char* assetsRoot, int removeExtension, char* output, unsigned long long outputSize);

/// @brief loads an image given a disk path using stb's library
CREN_API unsigned char* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels);

/// @brief release the stb image previously created
CREN_API void cren_stbimage_destroy(unsigned char* ptr);

/// @brief returns the last error from attempting to load image
CREN_API const char* cren_stbimage_get_error();

/// @brief loads a file given it's path
CREN_API unsigned int* cren_load_file(const char* path, unsigned long long* outSize);

#ifdef __cplusplus 
}
#endif

#endif // CREN_PLATFORM_INCLUDED