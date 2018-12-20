#include <Utils/Misc.h>

#include <Atrc/Lib/Core/Film.h>

namespace Atrc
{

FilmGrid::FilmGrid(const Recti &pixelRect, const FilmFilter &filter)
    : filter_(filter), pixelRect_(pixelRect),
      accValues_(pixelRect.GetDeltaX(), pixelRect.GetDeltaY(), Spectrum()),
      weights_(pixelRect.GetDeltaX(), pixelRect.GetDeltaY(), Spectrum())
{
    Vec2 r = filter.GetRadius();
    samplingRect_.low.x  = int32_t(std::floor(pixelRect.low.x + Real(0.5) - r.x));
    samplingRect_.low.y  = int32_t(std::floor(pixelRect.low.y + Real(0.5) - r.y));
    samplingRect_.high.x = int32_t(std::ceil(pixelRect.high.x - Real(0.5) + r.x));
    samplingRect_.high.y = int32_t(std::ceil(pixelRect.high.y - Real(0.5) + r.y));
}

FilmGrid::FilmGrid(FilmGrid &&moveFrom) noexcept
    : filter_(moveFrom.filter_),
      pixelRect_(moveFrom.pixelRect_), samplingRect_(moveFrom.samplingRect_),
      accValues_(std::move(moveFrom.accValues_)),
      weights_(std::move(moveFrom.weights_))
{

}

void FilmGrid::AddSample(const Vec2 &pos, const Spectrum &value) noexcept
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
            Spectrum w = filter_.Eval(pos.x - (x + Real(0.5)), pos.y - (y + Real(0.5)));
            int32_t lx = x - pixelRect_.low.x;
            accValues_(lx, ly) += w * value;
            weights_(lx, ly) += w;
        }
    }
}

Film::Film(const Vec2i &resolution, const FilmFilter &filter)
    : filter_(filter),
      accValues_(resolution.x, resolution.y),
      weights_(resolution.x, resolution.y),
      resolution_(resolution)
{

}

FilmGrid Film::CreateFilmGrid(const Recti &pixelRect) noexcept
{
    AGZ_ASSERT(pixelRect.low.x >= 0 && pixelRect.low.y >= 0);
    AGZ_ASSERT(pixelRect.high.x <= resolution_.x && pixelRect.high.y <= resolution_.y);
    return FilmGrid(pixelRect, filter_);
}

void Film::MergeFilmGrid(const FilmGrid &filmGrid) noexcept
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

Image Film::GetImage() const
{
    AGZ_ASSERT(accValues_.IsAvailable() && accValues_.GetSize() == weights_.GetSize());
    Image ret(accValues_.GetWidth(), accValues_.GetHeight());
    for(uint32_t y = 0; y < ret.GetHeight(); ++y)
    {
        for(uint32_t x = 0; x < ret.GetWidth(); ++x)
        {
            if(!!weights_(x, y))
                ret(x, y) = accValues_(x, y) / weights_(x, y);
        }
    }
    return ret;
}

} // namespace Atrc
