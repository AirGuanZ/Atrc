#pragma once

#include <algorithm>

#include <Atrc/ModelViewer/Console.h>

namespace Global
{
    struct GlobalContext
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        int menuBarHeight = 0;

        int renderingViewportHeight = 0;

        Console *console = nullptr;
    };

    inline GlobalContext gDefaultGlobalContext;
    inline GlobalContext *curCtx = &gDefaultGlobalContext;

    inline float GetWindowAspectRatio() noexcept
    {
        return static_cast<float>(curCtx->framebufferWidth) / curCtx->framebufferHeight;
    }

    inline float GetRenderingViewportAspectRatio() noexcept
    {
        return static_cast<float>(curCtx->framebufferWidth) / curCtx->renderingViewportHeight;
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
        curCtx->renderingViewportHeight = h - curCtx->menuBarHeight;
    }

    inline void _setMenuBarHeight(int h) noexcept
    {
        curCtx->menuBarHeight = h;
        curCtx->renderingViewportHeight = curCtx->framebufferHeight - h;
    }

    inline int GetRenderingViewportHeight() noexcept
    {
        return (std::max)(1, curCtx->renderingViewportHeight);
    }
}
