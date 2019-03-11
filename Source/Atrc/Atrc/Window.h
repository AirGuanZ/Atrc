#pragma once

class Window
{
    int FBW_ = 1, FBH_ = 1;
    float FBAspectRatio_ = 1;

public:

    int FBW() const noexcept { return FBW_; }
    int FBH() const noexcept { return FBH_; }
    float FBWf()  const noexcept { return static_cast<float>(FBW_); }
    float FBHf() const noexcept { return static_cast<float>(FBH_); }
    float FBAspectRatio() const noexcept { return FBAspectRatio_; }

    void SetFBSize(int w, int h)
    {
        FBW_ = w;
        FBH_ = h;
        FBAspectRatio_ = static_cast<float>(w) / h;
    }
};
