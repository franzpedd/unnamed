#include "Core/Renderer.h"
#include "Core/Application.h"

namespace Cosmos
{
    Renderer::Renderer(Application* app, const char* appName, unsigned int appVersion, bool customViewport, bool validations, bool vsync, const char* assetsPath, CRen_RendererAPI api, CRen_MSAA msaa)
        : mApp(app)
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
        ci.window = mApp->GetWindowRef()->GetNativeWindow(); // opaque pointer to underneath window
        ci.optionalHandle = mApp->GetWindowRef()->GetNativeOptionalHandle(); // opque pointer to underneath X11 Surface/Wayland Display

        mContext = cren_initialize(ci);
        CREN_ASSERT(mContext != nullptr, "Failed to create CRen Context");

        // set-up callbacks
        cren_set_user_pointer(mContext, this);

        cren_set_ui_image_count_callback(mContext, [](CRenContext* context, unsigned int count) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            //rendererClass.mApp->GetGUIRef().SetMinImageCount(count);
            });

        cren_set_draw_ui_raw_data_callback(mContext, [](CRenContext* context, void* commandbuffer) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            //rendererClass.mApp->GetGUIRef().DrawRawData(commandbuffer);
            });

        cren_set_resize_callback(mContext, [](CRenContext* context, unsigned int width, unsigned int height) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnResizeCallback(width, height);
            });

        cren_set_render_callback(mContext, [](CRenContext* context, CRen_RenderStage stage, float timestep) {
            Renderer& rendererClass = *(Renderer*)cren_get_user_pointer(context);
            rendererClass.OnRenderCallback(stage, timestep);
            });
    }

    Renderer::~Renderer()
    {
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
        //mApp->GetGUIRef().OnRender(stage);
    }

    void Renderer::OnResizeCallback(int width, int height)
    {

    }
}