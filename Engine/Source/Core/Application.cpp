#include "Application.h"

#include <chrono>
#include <thread>
#include <vecmath/vecmath.h>

namespace Cosmos
{
	Application::Application(const ApplicationCreateInfo& ci)
		: mApplicationCreateInfo(ci), mAssetsPath(ci.assetsPath)
	{
        mWindow = CreateUnique<Window>(this, ci.appName, ci.width, ci.height, ci.fullscreen, ci.assetsPath);
        mRenderer = CreateUnique<Renderer>(this, ci.appName, COSMOS_MAKE_VERSION(0, 1, 0, 0), ci.customViewport, ci.validations, ci.vsync, ci.assetsPath, ci.renderer, ci.msaa);
        mGUI = CreateUnique<GUI>(this);
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
            mGUI->OnUpdate();

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

        Shutdown();
	}

    void Application::Quit()
    {
        mWindow->Quit();
    }

    void Application::OnMinimize()
    {
        mRenderer->Minimize();
        mGUI->OnMinimize();
    }

    void Application::OnRestore(int width, int height)
    {
        mRenderer->Restore();
        mGUI->OnRestore(width, height);
    }

    void Application::OnResize(int width, int height)
    {
        if (!mGUI->WantToCaptureMouse()) {
            mRenderer->Resize(width, height);
            mGUI->OnResize(width, height);
        }
    }

    void Application::OnKeyPress(Input::Keycode keycode, Input::Keymod mod, bool held)
    {
        mGUI->OnKeyPress(keycode, mod, held);
        
        CRenCamera* cam = mRenderer->GetMainCamera();
        if (cren_camera_can_move(cam)) {
            if (keycode == Input::KEYCODE_W) cren_camera_move(cam, CREN_CAMERA_DIRECTION_FORWARD, true);
            if (keycode == Input::KEYCODE_S) cren_camera_move(cam, CREN_CAMERA_DIRECTION_BACKWARD, true);
            if (keycode == Input::KEYCODE_A) cren_camera_move(cam, CREN_CAMERA_DIRECTION_LEFT, true);
            if (keycode == Input::KEYCODE_D) cren_camera_move(cam, CREN_CAMERA_DIRECTION_RIGHT, true);
            if (keycode == Input::KEYCODE_LSHIFT) cren_camera_pressing_speed_modifier(cam, true);
        }
    }

    void Application::OnKeyRelease(Input::Keycode keycode)
    {
        mGUI->OnKeyRelease(keycode);
        
        CRenCamera* cam = mRenderer->GetMainCamera();
        if (cren_camera_can_move(cam)) {
            if (keycode == Input::KEYCODE_W) cren_camera_move(cam, CREN_CAMERA_DIRECTION_FORWARD, false);
            if (keycode == Input::KEYCODE_S) cren_camera_move(cam, CREN_CAMERA_DIRECTION_BACKWARD, false);
            if (keycode == Input::KEYCODE_A) cren_camera_move(cam, CREN_CAMERA_DIRECTION_LEFT, false);
            if (keycode == Input::KEYCODE_D) cren_camera_move(cam, CREN_CAMERA_DIRECTION_RIGHT, false);
            if (keycode == Input::KEYCODE_LSHIFT) cren_camera_pressing_speed_modifier(cam, false);
        }
    }

    void Application::OnButtonPress(Input::Buttoncode buttoncode, Input::Keymod mod)
    {
        mGUI->OnButtonPress(buttoncode, mod);
    }

    void Application::OnButtonRelease(Input::Buttoncode buttoncode)
    {
        mGUI->OnButtonRelease(buttoncode);
    }

    void Application::OnMouseScroll(double xoffset, double yoffset)
    {
        mGUI->OnMouseScroll(xoffset, yoffset);
    }

    void Application::OnMouseMove(double xpos, double ypos)
    {
        mGUI->OnMouseMove(xpos, ypos);
        
        CRenCamera* cam = mRenderer->GetMainCamera();
        if (cren_camera_can_move(cam)) {
            float3 rot = { float(-ypos), float(-xpos), 0.0f };
            cren_camera_rotate(cam, rot);
        }
    }

    void Application::OnDPIChange(float scale)
    {
        mGUI->OnDPIChange(scale);
    }
}