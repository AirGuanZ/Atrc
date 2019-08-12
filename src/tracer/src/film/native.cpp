#include <agz/tracer/core/film.h>
#include <agz/tracer/core/film_filter.h>

AGZ_TRACER_BEGIN

class NativeFilmGrid : public FilmGrid
{
    friend class NativeFilm;

    using value_data_t  = texture::texture2d_t<Spectrum>;
    using weight_data_t = texture::texture2d_t<real>;

    int grid_x_beg_;
    int grid_x_end_;
    int grid_y_beg_;
    int grid_y_end_;

    int sample_x_beg_;
    int sample_x_end_;
    int sample_y_beg_;
    int sample_y_end_;

    const FilmFilter *film_filter_;

    value_data_t values_;
    weight_data_t weights_;

    AlbedoBuffer   albedo_;
    PositionBuffer position_;
    NormalBuffer   normal_;
    DepthBuffer    depth_;

public:

    NativeFilmGrid(int x_beg, int x_end, int y_beg, int y_end, const FilmFilter *film_filter)
        : film_filter_(film_filter)
    {
        int x_size = x_end - x_beg;
        int y_size = y_end - y_beg;

        values_  = value_data_t(y_size, x_size);
        weights_ = weight_data_t(y_size, x_size);

        albedo_   = AlbedoBuffer(y_size, x_size);
        normal_   = NormalBuffer(y_size, x_size);
        position_ = PositionBuffer(y_size, x_size);
        depth_    = DepthBuffer(y_size, x_size);

        grid_x_beg_ = x_beg;
        grid_y_beg_ = y_beg;
        grid_x_end_ = x_end;
        grid_y_end_ = y_end;

        real radius = film_filter->radius();
        sample_x_beg_ = static_cast<int>(std::floor(grid_x_beg_ + real(0.5) - radius));
        sample_y_beg_ = static_cast<int>(std::floor(grid_y_beg_ + real(0.5) - radius));
        sample_x_end_ = static_cast<int>(std::ceil(grid_x_end_ - real(0.5) + radius));
        sample_y_end_ = static_cast<int>(std::ceil(grid_y_end_ - real(0.5) + radius));
    }

    int sample_x_beg() const noexcept override
    {
        return sample_x_beg_;
    }

    int sample_x_end() const noexcept override
    {
        return sample_x_end_;
    }

    int sample_y_beg() const noexcept override
    {
        return sample_y_beg_;
    }

    int sample_y_end() const noexcept override
    {
        return sample_y_end_;
    }

    void add_sample(const Vec2 &pos, const Spectrum &value, const GBufferPixel &gpixel, real w) override
    {
        real radius = film_filter_->radius();

        int x_beg = static_cast<int>(std::ceil(pos.x - radius - real(0.5)));
        int y_beg = static_cast<int>(std::ceil(pos.y - radius - real(0.5)));
        int x_end = static_cast<int>(std::floor(pos.x + radius - real(0.5))) + 1;
        int y_end = static_cast<int>(std::floor(pos.y + radius - real(0.5))) + 1;

        x_beg = std::max(grid_x_beg_, x_beg);
        y_beg = std::max(grid_y_beg_, y_beg);
        x_end = std::min(grid_x_end_, x_end);
        y_end = std::min(grid_y_end_, y_end);

        for(int y = y_beg; y < y_end; ++y)
        {
            int ly = y - grid_y_beg_;
            real y_cen = y + real(0.5);
            real y_rel = std::abs(pos.y - y_cen);
            if(y_rel > radius)
                continue;

            for(int x = x_beg; x < x_end; ++x)
            {
                int lx = x - grid_x_beg_;
                real x_cen = x + real(0.5);
                real x_rel = std::abs(pos.x - x_cen);
                if(x_rel > radius)
                    continue;

                real weight = w * film_filter_->eval(x_rel, y_rel);
                values_(ly, lx) += weight * value;
                weights_(ly, lx) += weight;

                albedo_(ly, lx)   += weight * gpixel.albedo;
                position_(ly, lx) += weight * gpixel.position;
                normal_(ly, lx)   += weight * gpixel.normal;
                depth_(ly, lx)    += weight * gpixel.depth;
            }
        }
    }
};

class NativeFilm : public Film
{
    int h_ = 0, w_ = 0;
    const FilmFilter *film_filter_ = nullptr;

    texture::texture2d_t<Spectrum> values_;
    texture::texture2d_t<real> weights_;

    // gbuffer attributes

    AlbedoBuffer   albedo_;
    PositionBuffer position_;
    NormalBuffer   normal_;
    DepthBuffer    depth_;

    std::mutex mut_;

public:

    using Film::Film;

    static std::string description()
    {
        return R"__(
native [Film]
    height [int] framebuffer height
    width  [int] framebuffer width
    filter [FilmFilter] film filter
        
    gbuffer is supported
        )__";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        h_ = params.child_int("height");
        w_ = params.child_int("width");
        if(h_ <= 0 || w_ <= 0)
            throw ObjectConstructionException("invalid film size");

        film_filter_ = FilmFilterFactory.create(params.child_group("filter"), init_ctx);

        values_ = texture::texture2d_t<Spectrum>(h_, w_);
        weights_ = texture::texture2d_t<real>(h_, w_);

        albedo_   = AlbedoBuffer(h_, w_);
        normal_   = NormalBuffer(h_, w_);
        position_ = PositionBuffer(h_, w_);
        depth_    = DepthBuffer(h_, w_);

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
            }
        }
    }

    std::unique_ptr<FilmGrid> new_grid(int x_beg, int x_end, int y_beg, int y_end) const override
    {
        return std::make_unique<NativeFilmGrid>(x_beg, x_end, y_beg, y_end, film_filter_);
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
                ret.albedo->at(y, x)   = ratio * albedo_(y, x);
                ret.position->at(y, x) = ratio * position_(y, x);
                ret.normal->at(y, x)   = ratio * normal_(y, x);
                ret.depth->at(y, x)    = ratio * depth_(y, x);
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
