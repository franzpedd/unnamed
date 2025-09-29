#include <stdio.h>

#include "Core/Editor.h"

int main(int argc, char** argv)
{
    Cosmos::ApplicationCreateInfo ci = { 0 };
    ci.appName = "Cosmos Editor";
    ci.customViewport = true;
	ci.validations = true;
	ci.fullscreen = false;
	ci.vsync = false;
	ci.width = 1366;
	ci.height = 768;
	#if defined(_WIN32) || defined(_WIN64)
	ci.assetsPath = "../data";
	#else
	ci.assetsPath = "data";
	#endif
	ci.renderer = CREN_RENDERER_API_VULKAN_1_1;
	ci.msaa = CREN_MSAA_X4;

    Cosmos::Editor editor(ci);
	editor.Run();
    return 0;
}