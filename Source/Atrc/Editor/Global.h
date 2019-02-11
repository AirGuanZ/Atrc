#pragma once

#include <algorithm>

#include <Atrc/Editor/Console.h>

namespace Global
{
    struct GlobalContext
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;

        Console *console = nullptr;
    };

    inline GlobalContext gDefaultGlobalContext;
    inline GlobalContext *curCtx = &gDefaultGlobalContext;

    inline float GetWindowAspectRatio() noexcept
    {
        return static_cast<float>(curCtx->framebufferWidth) / curCtx->framebufferHeight;
    }

    inline void ShowNormalMessage(std::string msg)
    {
        if(curCtx->console)
            curCtx->console->AddMessage(std::move(msg));
    }

    inline void ShowErrorMessage(std::string msg)
    {
        if(curCtx->console)
            curCtx->console->AddError(std::move(msg));
    }

    inline void _setConsole(Console *console) noexcept
    {
        curCtx->console = console;
    }

    inline int GetFramebufferWidth() noexcept
    {
        return curCtx->framebufferWidth;
    }

    inline int GetFramebufferHeight() noexcept
    {
        return curCtx->framebufferHeight;
    }

    inline void _setFramebufferWidth(int w) noexcept
    {
        curCtx->framebufferWidth = w;
    }

    inline void _setFramebufferHeight(int h) noexcept
    {
        curCtx->framebufferHeight = h;
    }
}
