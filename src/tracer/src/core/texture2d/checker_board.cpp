#include <agz/tracer/core/texture2d.h>
#include <agz-utils/misc.h>

AGZ_TRACER_BEGIN

class CheckerBoard : public Texture2D
{
    real grid_size_ = 1;

    FSpectrum color_1_ = FSpectrum(0, 0, 0);
    FSpectrum color_2_ = FSpectrum(1, 1, 1);

protected:

    FSpectrum sample_spectrum_impl(const Vec2& uv) const noexcept override
    {
        const int u_idx = static_cast<int>(math::saturate(uv.x) / grid_size_);
        const int v_idx = static_cast<int>(math::saturate(uv.y) / grid_size_);
        if((u_idx + v_idx) & 1)
            return color_1_;
        return color_2_;
    }

public:

    CheckerBoard(
        const Texture2DCommonParams &common_params,
        int grid_count, const FSpectrum &color1, const FSpectrum &color2)
    {
        init_common_params(common_params);

        if(grid_count <= 0)
            throw ObjectConstructionException(
                "invalid grid_count value: " + std::to_string(grid_count));
        grid_size_ = real(1) / grid_count;

        color_1_ = color1;
        color_2_ = color2;
    }

    int width() const noexcept override
    {
        return 1;
    }

    int height() const noexcept override
    {
        return 1;
    }
};

RC<Texture2D> create_checker_board(
    const Texture2DCommonParams &common_params,
    int grid_count, const FSpectrum &color1, const FSpectrum &color2)
{
    return newRC<CheckerBoard>(common_params, grid_count, color1, color2);
}

AGZ_TRACER_END
