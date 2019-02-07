#pragma once

#include <AGZUtils/Utils/Misc.h>

#include <Atrc/ModelViewer/Console.h>

struct Global : AGZ::Singleton<Global>
{
    int framebufferWidth;
    int framebufferHeight;

    Console *console = nullptr;

    float GetWindowAspectRatio() const noexcept
    {
        return static_cast<float>(framebufferWidth) / framebufferHeight;
    }

    static void ShowNormalMessage(std::string text)
    {
        auto &global = GetInstance();
        if(global.console)
            global.console->AddMessage(std::move(text));
    }

    static void ShowErrorMessage(std::string text)
    {
        auto &global = GetInstance();
        if(global.console)
            global.console->AddError(std::move(text));
    }
};
