#include <agz/tracer/core/film.h>
#include <agz/utility/misc.h>

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

    GBuffer gbuffer_;

public:

    NativeFilmGrid(int x_beg, int x_end, int y_beg, int y_end)
    {
        int x_size = x_end - x_beg;
        int y_size = y_end - y_beg;

        values_  = value_data_t(y_size, x_size);
        weights_ = weight_data_t(y_size, x_size);

        gbuffer_ = GBuffer(y_size, x_size);

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
        gbuffer_.albedo  ->at(ly, lx) += w * gpixel.albedo;
        gbuffer_.position->at(ly, lx) += w * gpixel.position;
        gbuffer_.normal  ->at(ly, lx) += w * gpixel.normal;
        gbuffer_.depth   ->at(ly, lx) += w * gpixel.depth;
        gbuffer_.binary  ->at(ly, lx) += w * gpixel.binary;
        weights_(ly, lx)  += w;
    }

    int x_size() const noexcept { return grid_x_end_ - grid_x_beg_; }

    int y_size() const noexcept { return grid_y_end_ - grid_y_beg_; }

    void clear()
    {
        values_.clear({});
        weights_.clear(0);
        gbuffer_.albedo->clear({});
        gbuffer_.position->clear({});
        gbuffer_.normal->clear({});
        gbuffer_.depth->clear(0);
        gbuffer_.binary->clear(0);
    }
};

class NativeFilm : public Film
{
    int h_ = 0, w_ = 0;

    value_data_t  values_;
    weight_data_t weights_;

    GBuffer gbuffer_;

public:

    void initialize(int width, int height)
    {
        AGZ_HIERARCHY_TRY

        if(width <= 0 || height <= 0)
            throw ObjectConstructionException("invalid film size");
        w_ = width;
        h_ = height;

        values_ = texture::texture2d_t<Spectrum>(h_, w_);
        weights_ = texture::texture2d_t<real>(h_, w_);

        gbuffer_ = GBuffer(h_, w_);

        AGZ_HIERARCHY_WRAP("in initializing native film")
    }

    void merge_grid(const FilmGrid &grid) override
    {
        auto &tgrid = dynamic_cast<const NativeFilmGrid&>(grid);

        for(int y = tgrid.grid_y_beg_; y < tgrid.grid_y_end_; ++y)
        {
            int ly = y - tgrid.grid_y_beg_;
            for(int x = tgrid.grid_x_beg_; x < tgrid.grid_x_end_; ++x)
            {
                int lx = x - tgrid.grid_x_beg_;
                values_(y, x)  += tgrid.values_(ly, lx);
                weights_(y, x) += tgrid.weights_(ly, lx);

                gbuffer_.albedo  ->at(y, x) += tgrid.gbuffer_.albedo  ->at(ly, lx);
                gbuffer_.position->at(y, x) += tgrid.gbuffer_.position->at(ly, lx);
                gbuffer_.normal  ->at(y, x) += tgrid.gbuffer_.normal  ->at(ly, lx);
                gbuffer_.depth   ->at(y, x) += tgrid.gbuffer_.depth   ->at(ly, lx);
                gbuffer_.binary  ->at(y, x) += tgrid.gbuffer_.binary  ->at(ly, lx);
            }
        }
    }

    std::unique_ptr<FilmGrid> new_grid(int x_beg, int x_end, int y_beg, int y_end) const override
    {
        return std::make_unique<NativeFilmGrid>(x_beg, x_end, y_beg, y_end);
    }

    std::unique_ptr<FilmGrid> renew_grid(int x_beg, int x_end, int y_beg, int y_end, std::unique_ptr<FilmGrid>&& old_grid) const override
    {
        auto *old = dynamic_cast<NativeFilmGrid*>(old_grid.get());
        int x_size = x_end - x_beg, y_size = y_end - y_beg;
        
        if(x_size != old->x_size() || y_size != old->y_size())
            return new_grid(x_beg, x_end, y_beg, y_end);

        old->clear();
        old->grid_x_beg_ = x_beg;
        old->grid_x_end_ = x_end;
        old->grid_y_beg_ = y_beg;
        old->grid_y_end_ = y_end;
        return std::move(old_grid);
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
                ret.albedo  ->at(y, x) = ratio * gbuffer_.albedo  ->at(y, x);
                ret.position->at(y, x) = ratio * gbuffer_.position->at(y, x);
                ret.normal  ->at(y, x) = ratio * gbuffer_.normal  ->at(y, x);
                ret.depth   ->at(y, x) = ratio * gbuffer_.depth   ->at(y, x);
                ret.binary  ->at(y, x) = ratio * gbuffer_.binary  ->at(y, x);
            }
        }
        return ret;
    }

    Vec2i resolution() const noexcept override
    {
        return { w_, h_ };
    }
};

std::shared_ptr<Film> create_native_film(
    int width, int height)
{
    auto ret = std::make_shared<NativeFilm>();
    ret->initialize(width, height);
    return ret;
}

AGZ_TRACER_END
