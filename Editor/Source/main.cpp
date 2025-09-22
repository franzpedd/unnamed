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
	ci.assetsPath = "../data";
	ci.renderer = CREN_RENDERER_API_VULKAN_1_1;
	ci.msaa = CREN_MSAA_X4;

    Cosmos::Editor editor(ci);
	editor.Run();
    return 0;
}