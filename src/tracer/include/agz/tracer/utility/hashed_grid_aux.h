#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

class HashedGridAux
{
public:

    // assert(entry_count % 8 == 0)
    HashedGridAux(
        const AABB &world_bound, real grid_sidelen, size_t entry_count);

    Vec3i pos_to_grid(const FVec3 &world_pos) const noexcept;

    size_t grid_to_entry(const Vec3i &grid) const noexcept;

    size_t pos_to_entry(const FVec3 &world_pos) const noexcept;

private:

    FVec3 world_low_;
    real grid_sidelen_;
    size_t entry_count_;
};

inline HashedGridAux::HashedGridAux(
    const AABB &world_bound, real grid_sidelen, size_t entry_count)
{
    assert(entry_count % 8 == 0);

    world_low_    = world_bound.low;
    grid_sidelen_ = grid_sidelen;
    entry_count_  = entry_count;
}

inline Vec3i HashedGridAux::pos_to_grid(const FVec3 &world_pos) const noexcept
{
    const auto hd = world_pos - world_low_;
    return {
        static_cast<int>(std::max<real>(hd.x, 0) / grid_sidelen_),
        static_cast<int>(std::max<real>(hd.y, 0) / grid_sidelen_),
        static_cast<int>(std::max<real>(hd.z, 0) / grid_sidelen_)
    };
}

inline size_t HashedGridAux::grid_to_entry(const Vec3i &grid) const noexcept
{
    // grids with neighboring x/y/z won't be mapped to the same entry index
    const size_t low3_bits =
        ((grid.x & 1) << 0) | ((grid.y & 1) << 1) | ((grid.z & 1) << 2);
    const size_t hash_val = misc::hash(grid.x, grid.y, grid.z);
    return ((hash_val << 3) % entry_count_) | low3_bits;
}

inline size_t HashedGridAux::pos_to_entry(const FVec3 &world_pos) const noexcept
{
    return grid_to_entry(pos_to_grid(world_pos));
}

AGZ_TRACER_END
