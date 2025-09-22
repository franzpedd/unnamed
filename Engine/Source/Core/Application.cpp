#include "Application.h"

#include <chrono>
#include <thread>
#include <vecmath/vecmath.h>

namespace Cosmos
{
	Application::Application(const ApplicationCreateInfo& ci)
		: mApplicationCreateInfo(ci)
	{
        mWindow = CreateUnique<Window>(this, ci.appName, ci.width, ci.height, ci.fullscreen, ci.assetsPath);
        mRenderer = CreateUnique<Renderer>(this, ci.appName, COSMOS_MAKE_VERSION(0, 1, 0, 0), ci.customViewport, ci.validations, ci.vsync, ci.assetsPath, ci.renderer, ci.msaa);
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<double>;

        TimePoint previousTime = Clock::now();
        double accumulator = 0.0;
        const double FIXED_TIMESTEP = 1.0 / 60.0;
        const int MAX_UPDATES = 8;

        // FPS tracking
        int frameCount = 0;
        double fpsAccumulator = 0.0;
        mAverageFPS = 0.0;

        while (!mWindow->ShouldClose())
        {
            TimePoint frameStart = Clock::now();

            // deltatime
            TimePoint currentTime = Clock::now();
            mTimeStep = std::chrono::duration_cast<Duration>(currentTime - previousTime).count();
            previousTime = currentTime;

            if (mTimeStep > 0.1) mTimeStep = 0.1; // clamp

            // update window events
            mWindow->OnUpdate();

            // accumulate time for fixed updates
            accumulator += mTimeStep;

            // fps tracking
            frameCount++;
            fpsAccumulator += mTimeStep;
            if (fpsAccumulator >= 1.0) {
                mAverageFPS = static_cast<double>(frameCount);
                frameCount = 0;
                fpsAccumulator -= 1.0;
            }

            // cap accumulator
            if (accumulator > FIXED_TIMESTEP * MAX_UPDATES) {
                accumulator = FIXED_TIMESTEP * MAX_UPDATES;
            }

            // fixed timestep updates
            int updateCount = 0;
            while (accumulator >= FIXED_TIMESTEP && updateCount < MAX_UPDATES) {
                mRenderer->OnUpdate(FIXED_TIMESTEP);
                accumulator -= FIXED_TIMESTEP;
                updateCount++;
            }

            // interpolation
            double alpha = accumulator / FIXED_TIMESTEP;

            // render
            mRenderer->OnRender(alpha);

            // automatic frame cap sleep
            TimePoint frameEnd = Clock::now();
            double frameDuration = std::chrono::duration_cast<Duration>(frameEnd - frameStart).count();

            if (!mRenderer->GetVSync()) {
                if (frameDuration < mTargetFrameTime) {
                    auto sleepTime = std::chrono::duration<double>(mTargetFrameTime - frameDuration);
                    std::this_thread::sleep_for(sleepTime);
                }
            }
        }

        // final FPS
        if (frameCount > 0 && fpsAccumulator > 0.0) {
            mAverageFPS = (double)(frameCount) / fpsAccumulator;
        }
	}

    void Application::Quit()
    {
        mWindow->Quit();
    }

    void Application::OnMinimize()
    {
        mRenderer->Minimize();
        //mGUI.OnMinimize();
    }

    void Application::OnRestore(int width, int height)
    {
        mRenderer->Restore();
        //mGUI.OnRestore(width, height);
    }

    void Application::OnResize(int width, int height)
    {
        //if (!mGUI.WantToCaptureMouse()) {
        //    mRenderer.Resize(width, height);
        //    mGUI.OnResize(width, height);
        //}
    }

    void Application::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
    {
        //mGUI.OnKeyPress(keycode, mod, held);
        //
        //// camera
        //CRenCamera& cam = mRenderer.GetContext()->camera;
        //if (cam.shouldMove) {
        //    if (keycode == Input::KEYCODE_W) cam.movingForward = 1;
        //    if (keycode == Input::KEYCODE_S) cam.movingBackward = 1;
        //    if (keycode == Input::KEYCODE_A) cam.movingLeft = 1;
        //    if (keycode == Input::KEYCODE_D) cam.movingRight = 1;
        //    if (keycode == Input::KEYCODE_LSHIFT) cam.modifierPressed = 1;
        //}
    }

    void Application::OnKeyRelease(Input::Keycode keycode)
    {
        //mGUI.OnKeyRelease(keycode);
        //
        //// camera
        //CRenCamera& cam = mRenderer.GetContext()->camera;
        //if (cam.shouldMove) {
        //    if (keycode == Input::KEYCODE_W) cam.movingForward = 0;
        //    if (keycode == Input::KEYCODE_S) cam.movingBackward = 0;
        //    if (keycode == Input::KEYCODE_A) cam.movingLeft = 0;
        //    if (keycode == Input::KEYCODE_D) cam.movingRight = 0;
        //    if (keycode == Input::KEYCODE_LSHIFT) cam.modifierPressed = 0;
        //}
    }

    void Application::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
    {
        //mGUI.OnButtonPress(buttoncode, mod);
    }

    void Application::OnButtonRelease(Input::Buttoncode buttoncode)
    {
        //mGUI.OnButtonRelease(buttoncode);
    }

    void Application::OnMouseScroll(double xoffset, double yoffset)
    {
        //mGUI.OnMouseScroll(xoffset, yoffset);
    }

    void Application::OnMouseMove(double xpos, double ypos)
    {
        //mGUI.OnMouseMove(xpos, ypos);
        //
        //// camera
        //CRenCamera& cam = mRenderer.GetContext()->camera;
        //if (cam.shouldMove) {
        //
        //    // avoid scene flip
        //    if (cam.rotation.x >= 89.0f) cam.rotation.x = 89.0f;
        //    if (cam.rotation.x <= -89.0f) cam.rotation.x = -89.0f;
        //
        //    // reset rotation on 360 degrees
        //    if (cam.rotation.x >= 360.0f) cam.rotation.x = 0.0f;
        //    if (cam.rotation.x <= -360.0f) cam.rotation.x = 0.0f;
        //    if (cam.rotation.y >= 360.0f) cam.rotation.y = 0.0f;
        //    if (cam.rotation.y <= -360.0f) cam.rotation.y = 0.0f;
        //
        //    float rotationspeed = 1.0f;
        //    float3 rot = { float(-ypos) * rotationspeed * 0.5f , float(xpos) * rotationspeed * 0.5f, 0.0f };
        //    cren_camera_rotate(&cam, rot);
        //}
        //
        //// mouse position
        //float2 mousePos = { 0.0f, 0.0f };
        //if (mAppCreateInfo.requestViewport) {
        //    CRenContext* context = mRenderer.GetContext();
        //    mousePos = mWindow.GetViewportCursorPosition(context->viewportPos, context->viewportSize);
        //}
        //
        //else {
        //    CRenContext* context = mRenderer.GetContext();
        //    mousePos = mWindow.GetCursorPosition();
        //}
    }

    void Application::OnDPIChange(float scale)
    {
        //mGUI.OnDPIChange(scale);
    }
}