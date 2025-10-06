#ifndef CREN_PLATFORM_INCLUDED
#define CREN_PLATFORM_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief detects what windowing/operating system is currently being used
CREN_API CRen_Platform cren_detect_platform();

/// @brief formats a const char* with a disk address of a file, constructing it's path
CREN_API void cren_get_path(const char* subpath, const char* assetsRoot, bool removeExtension, char* output, size_t outputSize);

/// @brief loads an image given a disk path using stb's library
CREN_API uint8_t* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels);

/// @brief release the stb image previously created
CREN_API void cren_stbimage_destroy(uint8_t* ptr);

/// @brief returns the last error from attempting to load image
CREN_API const char* cren_stbimage_get_error();

/// @brief loads a file given it's path
CREN_API unsigned int* cren_load_file(const char* path, size_t* outSize);

#ifdef __cplusplus 
}
#endif

#endif // CREN_PLATFORM_INCLUDED