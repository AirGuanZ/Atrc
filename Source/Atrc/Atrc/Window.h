#pragma once

class Window
{
    int fbW_ = 1, fbH_ = 1;
    float fbAspectRatio_ = 1;

    int pvW_ = 1, pvH_ = 1;
    float pvAspectRatio_ = 1;

public:

    int FbW() const noexcept { return fbW_; }
    int FbH() const noexcept { return fbH_; }
    float FbWf()  const noexcept { return static_cast<float>(fbW_); }
    float FbHf() const noexcept { return static_cast<float>(fbH_); }
    float FbAspectRatio() const noexcept { return fbAspectRatio_; }

    void SetFBSize(int w, int h) noexcept
    {
        AGZ_ASSERT(w > 0 && h > 0);
        fbW_ = w;
        fbH_ = h;
        fbAspectRatio_ = static_cast<float>(w) / h;
    }

    int PvW() const noexcept { return pvW_; }
    int PvH() const noexcept { return pvH_; }
    float PvWf() const noexcept { return static_cast<float>(pvW_); }
    float PvHf() const noexcept { return static_cast<float>(pvH_); }
    float PvAspectRatio() const noexcept { return pvAspectRatio_; }

    void SetPvSize(int w, int h) noexcept
    {
        AGZ_ASSERT(w > 0 && h > 0);
        pvW_ = w;
        pvH_ = h;
        pvAspectRatio_ = static_cast<float>(w) / h;
    }
};
