#pragma once

#include <atomic>
#include <vector>

#include <agz/tracer/core/bsdf.h>

AGZ_TRACER_BEGIN

namespace sppm
{

struct VisiblePoint
{
    Vec3 pos;
    Vec3 dir;
    Vec3 nor;
    Spectrum f;
    const BSDF *bsdf = nullptr;
};

struct PixelParams
{
    real R                   = 0;
    real N                   = 0;
    std::atomic<int> M       = 0;
    Spectrum tau;
    std::atomic<real> Phi[SPECTRUM_COMPONENT_COUNT] = { 0, 0, 0 };

    Spectrum direct_illu; // 存储直接命中光源产生的光照

    PixelParams() = default;
    PixelParams(const PixelParams &copy_from)
    {
        R      = copy_from.R;
        N      = copy_from.N;
        M      = copy_from.M.load();
        tau    = copy_from.tau;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            Phi[i] = copy_from.Phi[i].load();
        direct_illu = copy_from.direct_illu;
    }
    PixelParams &operator=(const PixelParams &copy_from)
    {
        R      = copy_from.R;
        N      = copy_from.N;
        M      = copy_from.M.load();
        tau    = copy_from.tau;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            Phi[i] = copy_from.Phi[i].load();
        direct_illu = copy_from.direct_illu;
        return *this;
    }

    void iterate(real alpha)
    {
        real nextN = N + alpha * M;
        real nextR;
        if(N + M < real(0.1))
            nextR = R;
        else
            nextR = R * std::sqrt(nextN / (N + M));

        real ratioR = nextR / R;
        Spectrum phi(Phi[0].load(), Phi[1].load(), Phi[2].load());
        Spectrum next_tau = (tau + phi) * ratioR * ratioR;

        N      = nextN;
        M      = 0;
        Phi[0] = 0;
        Phi[1] = 0;
        Phi[2] = 0;
        tau    = next_tau;
        R      = nextR;
    }
};

struct VPRec
{
    Vec3 pos;
    VisiblePoint *vp = nullptr;
    PixelParams  *pp = nullptr;

    bool contains(const Vec3 &p) const noexcept
    {
        return (pos - p).length_square() <= pp->R * pp->R;
    }
};

struct Grid
{
    std::vector<VPRec> vps;
};

class VPSearcher : public misc::uncopyable_t
{
    AABB world_bound_;
    Vec3 world_delta_;

    std::vector<Grid> grids_;
    Vec3 grid_size_;
    Vec3i grids_shape_;

    int to_linear(int xi, int yi, int zi) const
    {
        int ret = zi * grids_shape_.x * grids_shape_.y + yi * grids_shape_.x + xi;
        assert(0 <= ret && ret < static_cast<int>(grids_.size()));
        return ret;
    }

    AABB grid_bound(int xi, int yi, int zi) const
    {
        Vec3 low  = world_bound_.low + Vec3(real(xi), real(yi), real(zi)) * grid_size_;
        Vec3 high = low + grid_size_;
        return { low, high };
    }

    Vec3i grid_idx(const Vec3 &p) const
    {
        Vec3 canonical_p = (p - world_bound_.low) / world_delta_;
        int xi = math::clamp<int>(static_cast<int>(canonical_p.x * grids_shape_.x), 0, grids_shape_.x - 1);
        int yi = math::clamp<int>(static_cast<int>(canonical_p.y * grids_shape_.y), 0, grids_shape_.y - 1);
        int zi = math::clamp<int>(static_cast<int>(canonical_p.z * grids_shape_.z), 0, grids_shape_.z - 1);
        return { xi, yi, zi };
    }

    Grid *grid_contains(const Vec3 &p)
    {
        auto [xi, yi, zi] = grid_idx(p);
        return &grids_[to_linear(xi, yi, zi)];
    }

public:

    VPSearcher(const AABB &world_bound, const Vec3i &grids_shape)
        : world_bound_(world_bound), grids_shape_(grids_shape)
    {
        int grid_count = grids_shape.product();
        grids_.resize(grid_count);

        world_delta_ = world_bound.high - world_bound.low;
        grid_size_.x = world_delta_.x / grids_shape.x;
        grid_size_.y = world_delta_.y / grids_shape.y;
        grid_size_.z = world_delta_.z / grids_shape.z;
    }

    void add_vp(const VPRec &vp)
    {
        // slightly expand vp bound to avoid floating-point round error

        AABB vp_bound(vp.pos - Vec3(vp.pp->R), vp.pos + Vec3(vp.pp->R));
        vp_bound.low  -= real(0.01) * vp.pp->R;
        vp_bound.high += real(0.01) * vp.pp->R;

        Vec3i low_grid_idx  = grid_idx(vp_bound.low);
        Vec3i high_grid_idx = grid_idx(vp_bound.high);

        for(int zi = low_grid_idx.z; zi <= high_grid_idx.z; ++zi)
        {
            for(int yi = low_grid_idx.y; yi <= high_grid_idx.y; ++yi)
            {
                for(int xi = low_grid_idx.x; xi <= high_grid_idx.x; ++xi)
                {
                    auto [low, high] = grid_bound(xi, yi, zi);
                    if(math::test_sphere_aabb_collision(vp.pos, vp.pp->R, low, high))
                        grids_[to_linear(xi, yi, zi)].vps.push_back(vp);
                }
            }
        }
    }

    template<typename Func>
    void foreach_vp_contains(const Vec3 &p, const Func &func)
    {
        if(!world_bound_.contains(p))
            return;

        auto grid = grid_contains(p);
        for(auto &vp : grid->vps)
        {
            if(vp.contains(p))
                func(vp);
        }
    }
};

} // namespace sppm

AGZ_TRACER_END

