#include "Core/Renderer.h"
#include "Core/Application.h"

#include <functional>

namespace Cosmos
{
    Renderer::Renderer(Application* app, const char* appName, unsigned int appVersion, bool customViewport, bool validations, bool vsync, const char* assetsPath, CRen_RendererAPI api, CRen_MSAA msaa)
        : mApp(app), mAPI(api)
    {
        // initialization
        CRenCreateInfo ci = { 0 };
        ci.appName = appName;
        ci.assetsPath = assetsPath;
        ci.api = api;
        ci.appVersion = appVersion;
        ci.width = mApp->GetWindowRef()->GetWidth();
        ci.height = mApp->GetWindowRef()->GetHeight();
        ci.msaa = msaa;
        ci.validations = validations;
        ci.vsync = vsync;
        ci.customViewport = customViewport;

        mContext = cren_initialize(ci);
        CREN_ASSERT(mContext != nullptr, "Failed to create CRen Context");

        // set-up callbacks
        cren_set_user_pointer(mContext, this);

        cren_set_ui_image_count_callback(mContext, [](CRenContext* context, unsigned int count) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.mApp->GetGUIRef()->SetMinImageCount(count);
            });

        cren_set_draw_ui_raw_data_callback(mContext, [](CRenContext* context, void* commandbuffer) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.mApp->GetGUIRef()->DrawRawData(commandbuffer);
            });

        cren_set_resize_callback(mContext, [](CRenContext* context, unsigned int width, unsigned int height) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnResizeCallback(width, height);
            });

        cren_set_render_callback(mContext, [](CRenContext* context, CRen_RenderStage stage, float timestep) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnRenderCallback(stage, timestep);
            });

        cren_set_get_vulkan_instance_required_extensions_callback(mContext, [](CRenContext* context, uint32_t* count) -> const char* const* {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            
            static std::vector<const char*> extensions;
            extensions.clear();
        
            uint32_t baseCount = 0;
            const char* const* baseExtensions = rendererClass.mApp->GetWindowRef()->GetRequiredExtensions(&baseCount);
            
            for (uint32_t i = 0; i < baseCount; i++) extensions.push_back(baseExtensions[i]);
            
            if (cren_are_validations_enabled(context)) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            *count = extensions.size();
            return extensions.data();
            });


        cren_set_create_vulkan_surface_callback(mContext, [](CRenContext* context, void* instance, void* surface) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.mApp->GetWindowRef()->CreateSurface(instance, surface);
            });
        
        // initialize the renderer
        cren_create_renderer(mContext);

        mWorld = new World(mApp->GetRendererRef()); // loading a default world
    }

    Renderer::~Renderer()
    {
        mWorld->Destroy();
        cren_shutdown(mContext);
    }

    void Renderer::OnUpdate(float timestep)
    {
        cren_update(mContext, timestep);
    }

    void Renderer::OnRender(float timestep)
    {
        cren_render(mContext, timestep);
    }

    void Renderer::Minimize()
    {
        cren_minimize(mContext);
    }

    void Renderer::Restore()
    {
        cren_restore(mContext);
    }

    void Renderer::Resize(int width, int height)
    {
        cren_resize(mContext, width, height);
    }

    bool Renderer::GetVSync()
    {
        return cren_using_vsync(mContext);
    }

    CRenCamera* Renderer::GetMainCamera()
    {
        return cren_get_camera(mContext);
    }

    void Renderer::OnRenderCallback(int stage, double timestep)
    {
        //mApp->GetWorldRef().OnRender(stage);
        mApp->GetGUIRef()->OnRender(stage);
    }

    void Renderer::OnResizeCallback(int width, int height)
    {
        cren_resize(mContext, width, height);
    }
}