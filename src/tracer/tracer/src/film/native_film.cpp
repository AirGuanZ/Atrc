#include <agz/tracer/core/film.h>

AGZ_TRACER_BEGIN

using value_data_t  = texture::texture2d_t<Spectrum>;
using weight_data_t = texture::texture2d_t<real>;

class NativeFilmGrid : public FilmGrid
{
    friend class NativeFilm;

    int grid_x_beg_;
    int grid_x_end_;
    int grid_y_beg_;
    int grid_y_end_;

    value_data_t values_;
    weight_data_t weights_;

    AlbedoBuffer   albedo_;
    PositionBuffer position_;
    NormalBuffer   normal_;
    DepthBuffer    depth_;
    BinaryBuffer   binary_;

public:

    NativeFilmGrid(int x_beg, int x_end, int y_beg, int y_end)
    {
        int x_size = x_end - x_beg;
        int y_size = y_end - y_beg;

        values_  = value_data_t(y_size, x_size);
        weights_ = weight_data_t(y_size, x_size);

        albedo_   = AlbedoBuffer(y_size, x_size);
        normal_   = NormalBuffer(y_size, x_size);
        position_ = PositionBuffer(y_size, x_size);
        depth_    = DepthBuffer(y_size, x_size);
        binary_   = BinaryBuffer(y_size, x_size);

        grid_x_beg_ = x_beg;
        grid_y_beg_ = y_beg;
        grid_x_end_ = x_end;
        grid_y_end_ = y_end;
    }

    int sample_x_beg() const noexcept override
    {
        return grid_x_beg_;
    }

    int sample_x_end() const noexcept override
    {
        return grid_x_end_;
    }

    int sample_y_beg() const noexcept override
    {
        return grid_y_beg_;
    }

    int sample_y_end() const noexcept override
    {
        return grid_y_end_;
    }

    void add_sample(const Vec2 &pos, const Spectrum &value, const GBufferPixel &gpixel, real w) override
    {
        int x = static_cast<int>(std::floor(pos.x));
        int y = static_cast<int>(std::floor(pos.y));

        if(x < grid_x_beg_ || x >= grid_x_end_ || y < grid_y_beg_ || y >= grid_y_end_)
            return;

        int lx = x - grid_x_beg_, ly = y - grid_y_beg_;
        values_(ly, lx)   += w * value;
        albedo_(ly, lx)   += w * gpixel.albedo;
        position_(ly, lx) += w * gpixel.position;
        normal_(ly, lx)   += w * gpixel.normal;
        depth_(ly, lx)    += w * gpixel.depth;
        binary_(ly, lx)   += w * gpixel.binary;
        weights_(ly, lx)  += w;
    }
};

class NativeFilm : public Film
{
    int h_ = 0, w_ = 0;

    value_data_t  values_;
    weight_data_t weights_;

    AlbedoBuffer   albedo_;
    PositionBuffer position_;
    NormalBuffer   normal_;
    DepthBuffer    depth_;
    BinaryBuffer   binary_;

    std::mutex mut_;

public:

    using Film::Film;

    static std::string description()
    {
        return R"__(
native [Film]
    height [int] framebuffer height
    width  [int] framebuffer width
        
    gbuffer is supported
        )__";
    }
    
    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        h_ = params.child_int("height");
        w_ = params.child_int("width");
        if(h_ <= 0 || w_ <= 0)
            throw ObjectConstructionException("invalid film size");

        values_  = texture::texture2d_t<Spectrum>(h_, w_);
        weights_ = texture::texture2d_t<real>(h_, w_);

        albedo_   = AlbedoBuffer(h_, w_);
        normal_   = NormalBuffer(h_, w_);
        position_ = PositionBuffer(h_, w_);
        depth_    = DepthBuffer(h_, w_);
        binary_   = BinaryBuffer(h_, w_);

        AGZ_HIERARCHY_WRAP("in initializing native film object")
    }

    void merge_grid(FilmGrid &&grid) override
    {
        auto &tgrid = dynamic_cast<const NativeFilmGrid&>(grid);
        std::lock_guard<std::mutex> lk(mut_);

        for(int y = tgrid.grid_y_beg_; y < tgrid.grid_y_end_; ++y)
        {
            int ly = y - tgrid.grid_y_beg_;
            for(int x = tgrid.grid_x_beg_; x < tgrid.grid_x_end_; ++x)
            {
                int lx = x - tgrid.grid_x_beg_;
                values_(y, x)  += tgrid.values_(ly, lx);
                weights_(y, x) += tgrid.weights_(ly, lx);

                albedo_(y, x)   += tgrid.albedo_(ly, lx);
                position_(y, x) += tgrid.position_(ly, lx);
                normal_(y, x)   += tgrid.normal_(ly, lx);
                depth_(y, x)    += tgrid.depth_(ly, lx);
                binary_(y, x)   += tgrid.binary_(ly, lx);
            }
        }
    }

    void add_grid(FilmGrid &&grid, const Spectrum &weight) override
    {
        auto &tgrid = dynamic_cast<const NativeFilmGrid&>(grid);
        std::lock_guard<std::mutex> lk(mut_);

        for(int y = tgrid.grid_y_beg_; y < tgrid.grid_y_end_; ++y)
        {
            int ly = y - tgrid.grid_y_beg_;
            for(int x = tgrid.grid_x_beg_; x < tgrid.grid_x_end_; ++x)
            {
                int lx = x - tgrid.grid_x_beg_;

                real rhs_w = tgrid.weights_(ly, lx);
                Spectrum rhs = rhs_w ? weight * tgrid.values_(ly, lx) / rhs_w : Spectrum(0);

                values_(y, x) += weights_(y, x) * rhs;
            }
        }
    }

    std::unique_ptr<FilmGrid> new_grid(int x_beg, int x_end, int y_beg, int y_end) const override
    {
        return std::make_unique<NativeFilmGrid>(x_beg, x_end, y_beg, y_end);
    }

    texture::texture2d_t<Spectrum> image() const override
    {
        texture::texture2d_t<Spectrum> ret(h_, w_);
        for(int y = 0; y < h_; ++y)
        {
            for(int x = 0; x < w_; ++x)
            {
                real w = weights_(y, x);
                ret(y, x) = w ? values_(y, x) / w : Spectrum(0);
            }
        }
        return ret;
    }

    GBuffer gbuffer() const override
    {
        GBuffer ret(h_, w_);
        for(int y = 0; y < h_; ++y)
        {
            for(int x = 0; x < w_; ++x)
            {
                real w = weights_(y, x);
                real ratio = w ? 1 / w : 0;
                ret.albedo  ->at(y, x) = ratio * albedo_(y, x);
                ret.position->at(y, x) = ratio * position_(y, x);
                ret.normal  ->at(y, x) = ratio * normal_(y, x);
                ret.depth   ->at(y, x) = ratio * depth_(y, x);
                ret.binary  ->at(y, x) = ratio * binary_(y, x);
            }
        }
        return ret;
    }

    Vec2i resolution() const noexcept override
    {
        return { w_, h_ };
    }

    void map_spectrum(std::function<Spectrum(const Spectrum&)> func) override
    {
        values_.get_data() = values_.get_data().map(func);
    }

    void map_weight(std::function<real(real)> func) override
    {
        weights_.get_data() = weights_.get_data().map(func);
    }

    void end() override
    {
        // do nothing
    }
};

AGZT_IMPLEMENTATION(Film, NativeFilm, "native")

AGZ_TRACER_END
