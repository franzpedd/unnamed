#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 6272 6262 26827 26819 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#if defined(_WIN32)
#pragma warning(pop)
#endif