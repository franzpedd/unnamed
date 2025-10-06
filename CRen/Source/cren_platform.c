#include "cren_platform.h"
#include "cren_error.h"

#include <memm/memm.h>
#include <stb/stb_image.h>
#include <stdio.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32) || defined(_WIN64) || (defined(__linux__) && !defined(__ANDROID__))

/// @brief loads a file using standart fopen
static unsigned int* internal_cren_dekstop_load_file(const char* path, size_t* outSize)
{
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    const long file_size_long = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size_long <= 0 || file_size_long % sizeof(unsigned int) != 0) {
        fclose(file);
        return NULL;
    }

    const size_t file_size = (size_t)file_size_long;
    unsigned int* spirv_code = (unsigned int*)malloc(file_size);
    if (!spirv_code) {
        fclose(file);
        return NULL;
    }

    const size_t words_read = fread(spirv_code, sizeof(unsigned int),file_size / sizeof(unsigned int), file);
    fclose(file);

    if (words_read != file_size / sizeof(unsigned int)) {
        free(spirv_code);
        return NULL;
    }

    if (outSize) *outSize = file_size;
    return spirv_code;
}

#elif defined(__ANDROID__)

/// @brief loads a file using AAsset
#error "This will need fixing once I get there again";
static unsigned int* internal_cren_android_load_file(const char* path, size_t* outSize)
{
    if (!g_AssetManager) {
        CREN_LOG("[Android]: AssetManager is NULL, make sure CRen is compiled as a Shared Library and don't forget to call cren_android_assets_manager_init from Java-Side");
        return NULL;
    }

    AAsset* asset = AAssetManager_open(g_AssetManager, path, AASSET_MODE_BUFFER);
    if (!asset) {
        CREN_LOG("[Android]: The desired Asset with path %s does not exists", path);
        return NULL;
    }

    const size_t file_size = AAsset_getLength(asset);
    if (file_size == 0 || file_size % sizeof(unsigned int) != 0) {
        AAsset_close(asset);
        return NULL;
    }

    unsigned int* data = (unsigned int*)malloc(file_size, 1);
    if (!data) {
        AAsset_close(asset);
        return NULL;
    }

    if (AAsset_read(asset, data, file_size) != file_size) {
        free(data);
        AAsset_close(asset);
        return NULL;
    }

    AAsset_close(asset);

    if (outSize) *outSize = file_size;
    return data;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// external
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CREN_API CRen_Platform cren_detect_platform()
{
    #if defined(_WIN32)
        return CREN_PLATFORM_WINDOWS;
    #elif defined(__APPLE__) && defined(__MACH__)
        #if TARGET_OS_IPHONE
            return CREN_PLATFORM_IOS;
        #elif TARGET_OS_OSX
            return CREN_PLATFORM_MACOS;
        #else
            return CREN_PLATFORM_UNKNOWN_APPLE;
        #endif
    #elif defined(__ANDROID__)
        return CREN_PLATFORM_ANDROID;
    #elif defined(__linux__)
        return CREN_PLATFORM_LINUX;
    #else
        return CREN_PLATFORM_UNKNOWN;
    #endif
}

CREN_API void cren_get_path(const char* subpath, const char* assetsRoot, bool removeExtension, char* output, size_t outputSize)
{
    snprintf(output, outputSize, "%s/%s", assetsRoot, subpath);

    if (removeExtension) {
        char* lastDot = strrchr(output, '.'); // find the last '.' in the string
        if (lastDot) {
            *lastDot = '\0'; // truncate the string at the last '.'
        }
    }
}

CREN_API uint8_t* cren_stbimage_load_from_file(const char* path, int desiredChannels, int* outWidth, int* outHeight, int* outChannels)
{
    int x, y, channels = 0;
    stbi_uc* pixels = stbi_load(path, &x, &y, &channels, desiredChannels);

    *outWidth = x;
    *outHeight = y;
    *outChannels = channels;
    return pixels;
}

CREN_API void cren_stbimage_destroy(uint8_t* ptr)
{
    stbi_image_free(ptr);
}

CREN_API const char* cren_stbimage_get_error()
{
    return stbi_failure_reason();
}

CREN_API unsigned int* cren_load_file(const char* path, size_t* outSize)
{
    #if defined(__linux__) && defined(__ANDROID__)
        return internal_cren_android_load_file(path, outSize);
    #elif defined(_WIN32) || defined(_WIN64) || (defined(__linux__) && !defined(__ANDROID__))
        return internal_cren_dekstop_load_file(path, outSize);
    #elif defined(__APPLE__) && defined(__MACH__)
        #error "This will need checking for ios and probably be different from macos";
    #endif
}
