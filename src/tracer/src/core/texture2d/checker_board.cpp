#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class CheckerBoard : public Texture2D
{
    real grid_size_ = 1;

    Spectrum color_1_ = Spectrum(0, 0, 0);
    Spectrum color_2_ = Spectrum(1, 1, 1);

protected:

    Spectrum sample_spectrum_impl(const Vec2& uv) const noexcept override
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
        int grid_count, const Spectrum &color1, const Spectrum &color2)
    {
        init_common_params(common_params);

        if(grid_count <= 0)
            throw ObjectConstructionException("invalid grid_count value: " + std::to_string(grid_count));
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

std::shared_ptr<Texture2D> create_checker_board(
    const Texture2DCommonParams &common_params,
    int grid_count, const Spectrum &color1, const Spectrum &color2)
{
    return std::make_shared<CheckerBoard>(common_params, grid_count, color1, color2);
}

AGZ_TRACER_END
