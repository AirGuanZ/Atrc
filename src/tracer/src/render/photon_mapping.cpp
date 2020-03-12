#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/intersection.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/direct_illum.h>
#include <agz/tracer/render/photon_mapping.h>

AGZ_TRACER_RENDER_BEGIN

namespace sppm
{

VisiblePointSearcher::VisiblePointSearcher(
    const AABB &world_bound, int max_axis_grid_res)
{
    world_low_ = world_bound.low;

    // compute grid size & resolution

    const Vec3 extent = world_bound.high - world_bound.low;
    grid_size_ = extent.max_elem() / real(max_axis_grid_res);

    grid_res_.x = int(std::ceil(extent.x / grid_size_));
    grid_res_.y = int(std::ceil(extent.y / grid_size_));
    grid_res_.z = int(std::ceil(extent.z / grid_size_));

    total_grid_count_ = grid_res_.product();

    // create node entries

    node_entry_count_ = grid_res_.product();
    node_entries_ = newBox<std::atomic<VPNode*>[]>(node_entry_count_);
    grid_index_dot_ = {
        1,
        grid_res_.x,
        grid_res_.x * grid_res_.y
    };

    clear();
}

void VisiblePointSearcher::clear()
{
    for(int i = 0; i < node_entry_count_; ++i)
        node_entries_[i] = nullptr;
}

void VisiblePointSearcher::add_vp(Pixel &pixel, Arena &vp_node_arena)
{
    const Vec3i &min_grid = pos_to_grid(pixel.vp.pos - Vec3(pixel.radius));
    const Vec3i &max_grid = pos_to_grid(pixel.vp.pos + Vec3(pixel.radius));

    for(int z = min_grid.z; z <= max_grid.z; ++z)
    {
        for(int y = min_grid.y; y <= max_grid.y; ++y)
        {
            for(int x = min_grid.x; x <= max_grid.x; ++x)
            {
                const int entry_idx = grid_to_entry({ x, y, z });
                auto &entry = node_entries_[entry_idx];

                VPNode *new_node = vp_node_arena.create<VPNode>();
                new_node->pixel = &pixel;
                new_node->next  = entry;

                while(!entry.compare_exchange_weak(new_node->next, new_node))
                    ;
            }
        }
    }
}

void VisiblePointSearcher::add_photon(
    const Vec3 &photon_pos, const Spectrum &phi, const Vec3 &wr)
{
    const int entry_index = pos_to_entry(photon_pos);
    for(VPNode *node = node_entries_[entry_index]; node; node = node->next)
    {
        auto &pixel = *node->pixel;
        if(distance2(pixel.vp.pos, photon_pos) > pixel.radius * pixel.radius)
            continue;

        const Spectrum delta_phi = phi * pixel.vp.bsdf->eval(
            wr, pixel.vp.wr, TransMode::Radiance);

        if(!delta_phi.is_finite())
            continue;

        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            math::atomic_add(pixel.phi[i], delta_phi[i]);
        ++pixel.M;
    }
}

Vec3i VisiblePointSearcher::pos_to_grid(const Vec3 &pos) const noexcept
{
    const Vec3 offset = pos - world_low_;
    return {
        static_cast<int>(offset.x / grid_size_),
        static_cast<int>(offset.y / grid_size_),
        static_cast<int>(offset.z / grid_size_)
    };
}

int VisiblePointSearcher::grid_to_entry(const Vec3i &grid_index) const noexcept
{
    assert(grid_index.x >= 0 && grid_index.y >= 0 && grid_index.z >= 0);
    assert(grid_index.x < grid_res_.x);
    assert(grid_index.y < grid_res_.y);
    assert(grid_index.z < grid_res_.z);
    return dot(grid_index_dot_, grid_index);
}

int VisiblePointSearcher::pos_to_entry(const Vec3 &pos) const noexcept
{
    return grid_to_entry(pos_to_grid(pos));
}

Pixel::VisiblePoint tracer_vp(
    int max_fwd_depth, int direct_illum_spv,
    const Scene &scene, const Ray &r, const Spectrum &init_coef,
    Arena &arena, Sampler &sampler,
    GBufferPixel *gpixel, Spectrum &direct_illum)
{
    Spectrum coef = init_coef;

    Ray ray = r;
    for(int depth = 0; depth < max_fwd_depth; ++depth)
    {
        // find closest intersection and bsdf

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
        {
            if(auto env = scene.envir_light())
                direct_illum += coef * env->radiance(ray.o, ray.d);
            return { {}, {}, {}, nullptr };
        }

        const ShadingPoint shd = inct.material->shade(inct, arena);

        if(depth == 0)
        {
            // fill g-pixel

            if(gpixel)
            {
                gpixel->albedo  = shd.bsdf->albedo();
                gpixel->normal  = shd.shading_normal;
                gpixel->denoise = inct.entity->get_no_denoise_flag() ?
                                  real(0) : real(1);
            }

            // account for direct illum at first inct

            if(auto l = inct.entity->as_light())
            {
                direct_illum += coef * l->radiance(
                    inct.pos, inct.geometry_coord.z, inct.uv, inct.wr);
            }
        }

        // direct illumination from next vertex

        Spectrum sum_di;
        for(int i = 0; i < direct_illum_spv; ++i)
        {
            for(auto l : scene.lights())
                sum_di += mis_sample_light(scene, l, inct, shd, sampler);
            sum_di += mis_sample_bsdf(scene, inct, shd, sampler);
        }
        direct_illum += coef * sum_di / real(direct_illum_spv);

        // create vp?

        if(shd.bsdf->has_diffuse_component() || depth == max_fwd_depth - 1)
        {
            if(!coef.is_finite())
                return { {}, {}, {}, nullptr };

            Pixel::VisiblePoint vp;
            vp.pos  = inct.pos;
            vp.coef = coef;
            vp.bsdf = shd.bsdf;
            vp.wr   = inct.wr;

            return vp;
        }

        // sample bsdf to create next ray

        const auto bsdf_sample = shd.bsdf->sample(
            inct.wr, TransMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f)
            return { {}, {}, {}, nullptr };

        coef *= bsdf_sample.f / bsdf_sample.pdf
              * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    return { {}, {}, {}, nullptr };
}

void trace_photon(
    int min_depth, int max_depth, real cont_prob,
    VisiblePointSearcher &vp_searcher,
    const Scene &scene, Arena &arena, Sampler &sampler)
{
    // emit a photon

    auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
    if(!light)
        return;

    const auto emit = light->sample_emit(sampler.sample5());
    if(!emit.radiance)
        return;

    Spectrum coef = emit.radiance * std::abs(cos(emit.nor, emit.dir))
                  / (select_light_pdf * emit.pdf_pos * emit.pdf_dir);

    Ray ray(emit.pos, emit.dir, EPS);

    // trace the photon

    for(int depth = 1; depth <= max_depth; ++depth)
    {
        // apply RR strategy

        if(depth > min_depth)
        {
            if(sampler.sample1().u > cont_prob)
                return;
            coef /= cont_prob;
        }

        // find closest intersection

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
            return;

        // accumulate flux at visible points
        // ignore direct illumination
        if(depth > 1)
            vp_searcher.add_photon(inct.pos, coef, inct.wr);

        // sample bsdf to create next ray

        const ShadingPoint shd = inct.material->shade(inct, arena);
        const auto bsdf_sample = shd.bsdf->sample(
            inct.wr, TransMode::Importance, sampler.sample3());
        if(!bsdf_sample.f)
            return;

        coef *= bsdf_sample.f / bsdf_sample.pdf
              * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }
}

void update_pixel_params(real alpha, Pixel &pixel)
{
    assert(pixel.vp.is_valid());

    if(pixel.M > 0)
    {
        real new_N = pixel.N + alpha * pixel.M;
        real new_R = pixel.radius * std::sqrt(new_N / (pixel.N + pixel.M));

        Spectrum phi;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            phi[i] = pixel.phi[i];

        pixel.tau = (pixel.tau + pixel.vp.coef * phi) * (new_R * new_R)
                  / (pixel.radius * pixel.radius);

        pixel.N      = new_N;
        pixel.radius = new_R;
        pixel.M      = 0;

        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            pixel.phi[i] = 0;
    }

    pixel.vp.coef = Spectrum(0);
    pixel.vp.bsdf = nullptr;
}

Spectrum compute_pixel_radiance(
    int direct_illum_N, uint64_t photon_N, const Pixel &pixel)
{
    const Spectrum direct_illum = pixel.direct_illum / real(direct_illum_N);

    const real dem = photon_N * PI_r * pixel.radius * pixel.radius;
    const Spectrum photon_illum = pixel.tau / dem;

    return direct_illum + photon_illum;
}

} // namespace sppm

AGZ_TRACER_RENDER_END
