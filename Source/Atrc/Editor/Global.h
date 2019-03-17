#pragma once

#include <algorithm>

#include <Atrc/Editor/Console.h>

namespace Global
{
    struct GlobalContext
    {
        int framebufferWidth = 1;
        int framebufferHeight = 1;

        int previewWindowX = 0;
        int previewWindowY = 0;
        int previewWindowWidth = 1;
        int previewWindowHeight = 1;

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

    inline int FbW() noexcept { return curCtx->framebufferWidth; }

    inline int FbH() noexcept { return curCtx->framebufferHeight; }

    inline float FbWf() noexcept { return static_cast<float>(curCtx->framebufferWidth); }

    inline float FbHf() noexcept { return static_cast<float>(curCtx->framebufferHeight); }

    inline int PvX() noexcept { return curCtx->previewWindowX; }

    inline int PvY() noexcept { return curCtx->previewWindowY; }

    inline int PvW() noexcept { return curCtx->previewWindowWidth; }

    inline int PvH() noexcept { return curCtx->previewWindowHeight; }

    inline float PvXf() noexcept { return static_cast<float>(curCtx->previewWindowX); }

    inline float PvYf() noexcept { return static_cast<float>(curCtx->previewWindowY); }

    inline float PvWf() noexcept { return static_cast<float>(curCtx->previewWindowWidth); }

    inline float PvHf() noexcept { return static_cast<float>(curCtx->previewWindowHeight); }

    inline float PvAspectRatio() noexcept { return static_cast<float>(PvW()) / PvH(); }

    inline void _setFramebufferWidth(int w) noexcept
    {
        curCtx->framebufferWidth = w;
    }

    inline void _setFramebufferHeight(int h) noexcept
    {
        curCtx->framebufferHeight = h;
    }

    inline void _setPreviewWindow(int x, int y, int w, int h) noexcept
    {
        curCtx->previewWindowX      = x;
        curCtx->previewWindowY      = y;
        curCtx->previewWindowWidth  = w;
        curCtx->previewWindowHeight = h;
    }
}
