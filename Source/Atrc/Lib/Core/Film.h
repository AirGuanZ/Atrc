#pragma once

#include <Utils/Texture.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc
{

class FilmFilter
{
    Vec2 radius_;

public:

    explicit FilmFilter(const Vec2 &radius) noexcept;

    virtual ~FilmFilter() = default;

    // 取得像素坐标系中的有效半径
    Vec2 GetRadius() const noexcept;

    virtual Spectrum Eval(Real relX, Real relY) const noexcept = 0;
};

class FilmGrid
{
    const FilmFilter &filter_;

    Recti pixelRect_;
    Recti samplingRect_;

    AGZ::Texture2D<Spectrum> accValues_;
    AGZ::Texture2D<Spectrum> weights_;

public:

    friend class Film;

    FilmGrid(const Recti &pixelRect, const FilmFilter &filter);

    FilmGrid(FilmGrid &&moveFrom) noexcept;

    Recti GetSamplingRect() const noexcept;

    void AddSample(const Vec2 &pos, const Spectrum &value) noexcept;
};

class Film
{
    const FilmFilter &filter_;

    AGZ::Texture2D<Spectrum> accValues_;
    AGZ::Texture2D<Spectrum> weights_;

    Vec2i resolution_;

public:

    Film(const Vec2i &resolution, const FilmFilter &filter);

    Vec2i GetResolution() const noexcept;

    FilmGrid CreateFilmGrid(const Recti &pixelRect) noexcept;

    void MergeFilmGrid(const FilmGrid &filmGrid) noexcept;

    Image GetImage() const;
};

// ================================= Implementation

inline FilmFilter::FilmFilter(const Vec2 &radius) noexcept
    : radius_(radius)
{

}

inline Vec2 FilmFilter::GetRadius() const noexcept
{
    return radius_;
}

inline Recti FilmGrid::GetSamplingRect() const noexcept
{
    return samplingRect_;
}

inline Vec2i Film::GetResolution() const noexcept
{
    return resolution_;
}

} // namespace Atrc
