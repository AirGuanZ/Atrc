#pragma once

#include <AGZUtils/Utils/Texture.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class FilmFilter
{
    Vec2 radius_;

public:

    explicit FilmFilter(const Vec2 &radius) noexcept : radius_(radius) { }

    virtual ~FilmFilter() = default;

    // 取得像素坐标系中的有效半径
    Vec2 GetRadius() const noexcept { return radius_; }

    virtual Real Eval(Real relX, Real relY) const noexcept = 0;
};

template<typename P>
class TFilmGrid
{
    const FilmFilter &filter_;

    Recti pixelRect_;
    Recti samplingRect_;

    AGZ::Texture2D<P> accValues_;
    AGZ::Texture2D<Real> weights_;

public:

    template<typename>
    friend class TFilm;

    TFilmGrid(const Recti &pixelRect, const FilmFilter &filter)
        : filter_(filter), pixelRect_(pixelRect),
          accValues_(pixelRect.GetDeltaX(), pixelRect.GetDeltaY()),
          weights_(pixelRect.GetDeltaX(), pixelRect.GetDeltaY())
    {
        Vec2 r = filter.GetRadius();
        samplingRect_.low.x  = int32_t(std::floor(pixelRect.low.x + Real(0.5) - r.x));
        samplingRect_.low.y  = int32_t(std::floor(pixelRect.low.y + Real(0.5) - r.y));
        samplingRect_.high.x = int32_t(std::ceil(pixelRect.high.x - Real(0.5) + r.x));
        samplingRect_.high.y = int32_t(std::ceil(pixelRect.high.y - Real(0.5) + r.y));
    }

    TFilmGrid(TFilmGrid<P> &&moveFrom) noexcept
        : filter_(moveFrom.filter_),
          pixelRect_(moveFrom.pixelRect_), samplingRect_(moveFrom.samplingRect_),
          accValues_(std::move(moveFrom.accValues_)),
          weights_(std::move(moveFrom.weights_))
    {

    }

    Recti GetSamplingRect() const noexcept { return samplingRect_; }

    void AddSample(const Vec2 &pos, const P &value) noexcept
    {
        Vec2 r = filter_.GetRadius();

        auto xBegin = Max<int32_t>(pixelRect_.low.x,  int32_t(std::ceil (pos.x - r.x - Real(0.5))));
        auto yBegin = Max<int32_t>(pixelRect_.low.y,  int32_t(std::ceil (pos.y - r.y - Real(0.5))));
        auto xEnd   = Min<int32_t>(pixelRect_.high.x, int32_t(std::floor(pos.x + r.x - Real(0.5)) + 1));
        auto yEnd   = Min<int32_t>(pixelRect_.high.y, int32_t(std::floor(pos.y + r.y - Real(0.5)) + 1));

        for(int32_t y = yBegin; y < yEnd; ++y)
        {
            int32_t ly = y - pixelRect_.low.y;
            for(int32_t x = xBegin; x < xEnd; ++x)
            {
                Vec2 rel = pos - (Vec2(Real(x), Real(y)) + Vec2(Real(0.5)));
                if(Abs(rel.x) > r.x || Abs(rel.y) > r.y)
                    continue;
                Real w = filter_.Eval(pos.x - (x + Real(0.5)), pos.y - (y + Real(0.5)));
                int32_t lx = x - pixelRect_.low.x;
                accValues_(lx, ly) += w * value;
                weights_(lx, ly) += w;
            }
        }
    }
};

template<typename P>
class TFilm
{
    const FilmFilter &filter_;

    AGZ::Texture2D<P> accValues_;
    AGZ::Texture2D<Real> weights_;

    Vec2i resolution_;

public:

    TFilm(const Vec2i &resolution, const FilmFilter &filter)
        : filter_(filter),
        accValues_(resolution.x, resolution.y),
        weights_(resolution.x, resolution.y),
        resolution_(resolution)
    {

    }

    Vec2i GetResolution() const noexcept { return resolution_; }

    TFilmGrid<P> CreateFilmGrid(const Recti &pixelRect) noexcept
    {
        AGZ_ASSERT(pixelRect.low.x >= 0 && pixelRect.low.y >= 0);
        AGZ_ASSERT(pixelRect.high.x <= resolution_.x && pixelRect.high.y <= resolution_.y);
        return TFilmGrid<P>(pixelRect, filter_);
    }

    void MergeFilmGrid(const TFilmGrid<P> &filmGrid) noexcept
    {
        for(int32_t y = filmGrid.pixelRect_.low.y;
            y < filmGrid.pixelRect_.high.y; ++y)
        {
            int32_t ly = y - filmGrid.pixelRect_.low.y;
            for(int32_t x = filmGrid.pixelRect_.low.x;
                x < filmGrid.pixelRect_.high.x; ++x)
            {
                int32_t lx = x - filmGrid.pixelRect_.low.x;
                accValues_(x, y) += filmGrid.accValues_(lx, ly);
                weights_(x, y) += filmGrid.weights_(lx, ly);
            }
        }
    }

    auto GetImage() const
    {
        AGZ_ASSERT(accValues_.IsAvailable() && accValues_.GetSize() == weights_.GetSize());
        AGZ::Texture2D<P> ret(accValues_.GetWidth(), accValues_.GetHeight());
        for(uint32_t y = 0; y < ret.GetHeight(); ++y)
        {
            for(uint32_t x = 0; x < ret.GetWidth(); ++x)
            {
                if(!!weights_(x, y))
                    ret(x, y) = P(accValues_(x, y) / weights_(x, y));
            }
        }
        return ret;
    }
};

} // namespace Atrc
