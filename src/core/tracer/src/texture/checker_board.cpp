#include <agz/tracer/core/texture.h>

AGZ_TRACER_BEGIN

class CheckerBoard : public Texture
{
    real grid_size_ = 1;

    Spectrum color_1_ = Spectrum(0, 0, 0);
    Spectrum color_2_ = Spectrum(1, 1, 1);

protected:

    Spectrum sample_spectrum_impl(const Vec2& uv) const noexcept override
    {
        int u_idx = static_cast<int>(uv.x / grid_size_);
        int v_idx = static_cast<int>(uv.y / grid_size_);
        if((u_idx + v_idx) & 1)
            return color_1_;
        return color_2_;
    }

public:

    using Texture::Texture;

    static std::string description()
    {
        return R"___(
checker_board [Texture]
    grid_count [real] (optional) grid count in each uv axis
    grid_size  [real] (required only when 'grid_count' is not specified) 1 / grid_count
    color_1    [Spectrum] (optional; defaultly set to 0)
    color_1    [Spectrum] (optional; defaultly set to 1)
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        init_transform(params);

        if(auto node = params.find_child("grid_count"))
            grid_size_ = 1 / node->as_value().as_real();
        else
            grid_size_ = params.child_real("grid_size");

        if(grid_size_ <= 0)
            throw ObjectConstructionException("invalid grid count/size value");

        if(params.find_child("color_1"))
            color_1_ = params.child_spectrum("color_1");

        if(params.find_child("color_2"))
            color_2_ = params.child_spectrum("color_2");

        AGZ_HIERARCHY_WRAP("in initializing checker_board texture object")
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

AGZT_IMPLEMENTATION(Texture, CheckerBoard, "checker_board")

AGZ_TRACER_END
