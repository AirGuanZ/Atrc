#pragma once

#include <agz/tracer/render/common.h>

AGZ_TRACER_RENDER_BEGIN

template<typename T>
class HashGrid
{
public:

    void set_grid_count(size_t count);
    
    void build(const T *points, size_t point_count, float radius);

    template<typename Func>
    void query(
        const T *points, size_t point_count,
        const Vec3 &query_pos, const Func &func);

private:

    Vec2i get_grid_range(int index) const;

    int get_grid_index(const Vec3 &pos) const;

    int get_grid_index(const Vec3i &index) const;

    Vec3 lower_;
    Vec3 upper_;

    std::vector<int> indices_;
    std::vector<int> grids_;

    float radius_  = 0;
    float radius2_ = 0;

    float grid_size_     = 1;
    float inv_grid_size_ = 1;
};

template<typename T>
void HashGrid<T>::set_grid_count(size_t count)
{
    grids_.resize(count);
}

template<typename T>
void HashGrid<T>::build(const T *points, size_t point_count, float radius)
{
    radius_  = radius;
    radius2_ = math::sqr(radius);

    grid_size_     = radius * 2;
    inv_grid_size_ = 1 / grid_size_;

    lower_ = Vec3(+REAL_INF);
    upper_ = Vec3(-REAL_INF);

    indices_.resize(point_count);
    std::fill(grids_.begin(), grids_.end(), 0);

    for(size_t i = 0; i < point_count; ++i)
    {
        const Vec3 pos = points[i].pos;
        lower_ = vec_min(lower_, pos);
        upper_ = vec_max(upper_, pos);
        ++grids_[get_grid_index(pos)];
    }

    // prefix sum of grids

    int sum = 0;
    for(size_t i = 0; i < grids_.size(); ++i)
    {
        int old = grids_[i];
        grids_[i] = sum;
        sum += old;
    }

    for(size_t i = 0; i < point_count; ++i)
    {
        const Vec3 pos = points[i].pos;
        const int index = grids_[get_grid_index(pos)]++;
        indices_[index] = static_cast<int>(i);
    }
}

template<typename T>
template<typename Func>
void HashGrid<T>::query(
    const T *points, size_t point_count,
    const Vec3 &query_pos, const Func &query)
{
    const Vec3 lower_dist = query_pos - lower_;
    const Vec3 upper_dist = upper_ - query_pos;
    if(lower_dist.min_elem() < 0 || upper_dist.min_elem() < 0)
        return;

    const Vec3 grid_pt = inv_grid_size_ * lower_dist;

    const Vec3 index_floor = {
        std::floor(grid_pt.x),
        std::floor(grid_pt.y),
        std::floor(grid_pt.z),
    };

    const Vec3i index_int = {
        static_cast<int>(index_floor.x),
        static_cast<int>(index_floor.y),
        static_cast<int>(index_floor.z),
    };

    const Vec3i index_offseted = index_int + (grid_pt - index_floor).map(
        [](float v) { return v < 0.5f ? -1 : 1; });

    for(int i = 0; i < 8; ++i)
    {
        Vec3i grid = index_int;
        if(i & 0b100) grid.x = index_offseted.x;
        if(i & 0b010) grid.y = index_offseted.y;
        if(i & 0b001) grid.z = index_offseted.z;

        const Vec2i grid_range = get_grid_range(get_grid_index(grid));

        for(int j = grid_range.x; j < grid_range.y; ++j)
        {
            const int pi = indices_[j];
            const float dist2 = (query_pos - points[pi].pos).length_square();

            if(dist2 <= radius2_)
                func(points[pi]);
        }
    }
}

template<typename T>
Vec2i HashGrid<T>::get_grid_range(int index) const
{
    if(!index)
        return { 0, grids_[0] };
    return { grids_[index - 1], grids_[index] };
}

template<typename T>
int HashGrid<T>::get_grid_index(const Vec3 &pos) const
{
    const Vec3 lower_dist = pos - lower_;
    const Vec3 index = {
        static_cast<int>(std::floor(inv_grid_size_ * lower_dist.x)),
        static_cast<int>(std::floor(inv_grid_size_ * lower_dist.y)),
        static_cast<int>(std::floor(inv_grid_size_ * lower_dist.z)),
    };
    return get_grid_index(index);
}

template<typename T>
int HashGrid<T>::get_grid_index(const Vec3i &index) const
{
    const auto x = static_cast<unsigned int>(index.x);
    const auto y = static_cast<unsigned int>(index.y);
    const auto z = static_cast<unsigned int>(index.z);
    return static_cast<int>(
        ((x * 73856093) ^ (y * 19349663) ^ (z * 83492791)) %
        static_cast<unsigned int>(grids_.size()));
}

AGZ_TRACER_RENDER_END
