#include <agz/tracer/create/renderer.h>
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/renderer_interactor.h>
#include <agz/tracer/core/render_target.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/direct_illum.h>
#include <agz/tracer/render/path_tracing.h>
#include <agz/tracer/utility/reservoir.h>
#include <agz-utils/thread.h>

AGZ_TRACER_BEGIN

class ReSTIRGIRenderer : public Renderer
{
    using ThreadPool = thread::thread_group_t;

    struct ReservoirData
    {
        Vec3        sample_point;
        Vec3        sample_normal;
        Vec3        sample_wi;
        Spectrum    sample_incident_radiance;
        const BSDF *sample_bsdf = nullptr;
    };

    struct Pixel
    {
        Spectrum accu_value;
        Spectrum accu_albedo;
        Vec3     accu_normal;
        real     accu_weight  = 0;
        real     accu_denoise = 0;

        Vec3        visible_wr;
        Vec3        visible_point;
        real        visible_dist = 0;
        Vec3        visible_normal;
        const BSDF *visible_bsdf = nullptr;

        Reservoir<ReservoirData> reservoir_a;
        Reservoir<ReservoirData> reservoir_b;
    };
    
    Spectrum trace(
        const render::TraceParams &params, const Scene &scene, const Ray &ray,
        Sampler &sampler, Arena &arena) const
    {
        FSpectrum coef(1);
        Ray r = ray;

        Spectrum pixel;

        for(int depth = 1, s_depth = 1; depth <= params.max_depth; ++depth)
        {
            // apply RR strategy

            if(depth > params.min_depth)
            {
                if(sampler.sample1().u > params.cont_prob)
                    return pixel;
                coef /= params.cont_prob;
            }

            // find closest entity intersection

            EntityIntersection ent_inct;
            const bool has_ent_inct = scene.closest_intersection(r, &ent_inct);
            if(!has_ent_inct)
                return pixel;
            
            const ShadingPoint ent_shd = ent_inct.material->shade(ent_inct, arena);

            // direct illumination

            FSpectrum direct_illum;
            for(int i = 0; i < params.direct_illum_sample_count; ++i)
            {
                for(auto light : scene.lights())
                {
                    direct_illum += coef * mis_sample_light(
                        scene, light, ent_inct, ent_shd, sampler);
                }
                direct_illum += coef * mis_sample_bsdf(
                    scene, ent_inct, ent_shd, sampler);
            }

            pixel += real(1) / params.direct_illum_sample_count * direct_illum;

            // sample bsdf

            auto bsdf_sample = ent_shd.bsdf->sample(
                ent_inct.wr, TransMode::Radiance, sampler.sample3());
            if(!bsdf_sample.f || bsdf_sample.pdf < EPS())
                return pixel;

            bool is_new_sample_delta = bsdf_sample.is_delta;
            AGZ_SCOPE_EXIT{
                if(is_new_sample_delta && depth >= 2 && s_depth <= params.specular_depth)
                {
                    --depth;
                    ++s_depth;
                }
            };

            const real abscos = std::abs(cos(
                ent_inct.geometry_coord.z, bsdf_sample.dir));
            coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;

            r = Ray(ent_inct.eps_offset(bsdf_sample.dir),
                    bsdf_sample.dir.normalize());
        }

        return pixel;
    }

    template<bool Vis>
    Spectrum eval(
        const Scene         &scene,
        const Pixel         &pixel,
        const ReservoirData &resData) const
    {
        assert(pixel.visible_bsdf);
        assert(resData.sample_bsdf);

        if constexpr(Vis)
        {
            if(!scene.visible(pixel.visible_point, resData.sample_point))
                return {};
        }

        if(!resData.sample_wi)
            return {};

        const Vec3 vp_to_sp = resData.sample_point - pixel.visible_point;

        const Spectrum f_vp = pixel.visible_bsdf->eval(
            vp_to_sp, pixel.visible_wr, TransMode::Radiance);

        const real absdot_vp = std::abs(cos(vp_to_sp, pixel.visible_normal));

        const Spectrum f_sp = resData.sample_bsdf->eval(
            resData.sample_wi, -vp_to_sp, TransMode::Radiance);

        return f_vp * absdot_vp * f_sp * resData.sample_incident_radiance;
    }

    template<bool Vis>
    real target_pdf(
        const Scene         &scene,
        const Pixel         &pixel,
        const ReservoirData &res_data) const
    {
        assert(pixel.visible_bsdf);
        assert(resData.sample_bsdf);

        if constexpr(Vis)
        {
            if(!scene.visible(pixel.visible_point, res_data.sample_point))
                return 0;
        }

        if(!res_data.sample_wi)
            return 0;

        const Vec3 vp_to_sp = res_data.sample_point - pixel.visible_point;
        
        const Spectrum f_sp = res_data.sample_bsdf->eval(
            res_data.sample_wi, -vp_to_sp, TransMode::Radiance);

        return f_sp.lum() * res_data.sample_incident_radiance.lum();
    }

    void create_initial_sample_for_pixel(
        const Vec2    &inv_res,
        const Scene   &scene,
        const Vec2i   &pixel_coord,
        Pixel         &pixel,
        NativeSampler &sampler,
        Arena         &frame_arena,
        Arena         &temp_arena) const
    {
        pixel.visible_bsdf                 = nullptr;
        pixel.reservoir_a.data.sample_bsdf = nullptr;
        pixel.reservoir_b.data.sample_bsdf = nullptr;
        pixel.reservoir_a.clear();

        pixel.accu_weight += 1;

        // build camera ray

        const Sample2 pixel_sam = sampler.sample2();
        const Vec2 film_coord = inv_res * Vec2(
            pixel_coord.x + pixel_sam.u, pixel_coord.y + pixel_sam.v);

        const CameraSampleWeResult cam_sam = scene.get_camera()->sample_we(
            film_coord, sampler.sample2());
        Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

        // find visible point

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
        {
            if(auto env = scene.envir_light())
            {
                const Spectrum env_rad = env->radiance(ray.o, ray.d);
                pixel.accu_value += cam_sam.throughput * env_rad;
            }
            return;
        }

        const auto first_shader = inct.material->shade(inct, frame_arena);

        pixel.accu_albedo  += first_shader.bsdf->albedo();
        pixel.accu_normal  += first_shader.shading_normal;
        pixel.accu_denoise += inct.entity->get_no_denoise_flag() ? real(0) : real(1);

        pixel.visible_wr     = -ray.d;
        pixel.visible_bsdf   = first_shader.bsdf;
        pixel.visible_point  = inct.pos;
        pixel.visible_dist   = distance(inct.pos, ray.o);
        pixel.visible_normal = inct.geometry_coord.z;

        // handle emission at vp

        if(auto light = inct.entity->as_light())
        {
            const Spectrum light_rad = light->radiance(
                inct.pos, inct.geometry_coord.z, inct.uv, inct.wr);
            pixel.accu_value += cam_sam.throughput * light_rad;
        }

        // estimate direct illum at vp

        Spectrum di = mis_sample_bsdf(scene, inct, first_shader, sampler);
        for(auto light : scene.lights())
        {
            di += mis_sample_light(
                scene, light, inct, first_shader, sampler);
        }
        if(di.is_finite())
            pixel.accu_value += di;

        // sample dir at vp

        const Sample2 vp_sam = sampler.sample2();
        const auto [local_vp_dir, vp_dir_pdf] = math::distribution
            ::uniform_on_hemisphere(vp_sam.u, vp_sam.v);

        const Vec3 global_vp_dir =
            inct.geometry_coord.local_to_global(local_vp_dir);
        const Ray vp_ray(inct.eps_offset(global_vp_dir), global_vp_dir);

        // find sample point

        if(!scene.closest_intersection(vp_ray, &inct))
        {
            pixel.reservoir_a.data.sample_bsdf = nullptr;
            return;
        }
        
        const auto second_shader = inct.material->shade(inct, frame_arena);

        pixel.reservoir_a.data.sample_point  = inct.pos;
        pixel.reservoir_a.data.sample_normal = inct.geometry_coord.z;
        pixel.reservoir_a.data.sample_bsdf   = second_shader.bsdf;

        // sample bsdf at sp

        const Sample2 sp_sam = sampler.sample2();
        const auto [local_sp_dir, sp_dir_pdf] = math::distribution
            ::uniform_on_hemisphere(sp_sam.u, sp_sam.v);

        const Vec3 global_sp_dir =
            inct.geometry_coord.local_to_global(local_sp_dir);

        const Spectrum sp_f = second_shader.bsdf->eval(
            global_sp_dir, inct.wr, TransMode::Radiance);

        if(!sp_f)
        {
            pixel.reservoir_a.data.sample_wi                = {};
            pixel.reservoir_a.data.sample_incident_radiance = {};

            pixel.reservoir_a.M    = 1;
            pixel.reservoir_a.wsum = 0;
            pixel.reservoir_a.W    = 1 / vp_dir_pdf;

            return;
        }

        const Ray sample_ray(inct.eps_offset(global_sp_dir), global_sp_dir);

        const Spectrum sample_li = trace_std(
            trace_params_, scene, sample_ray, sampler, temp_arena).value;

        const real sample_absdot =
            std::abs(cos(inct.geometry_coord.z, global_sp_dir));

        pixel.reservoir_a.data.sample_wi = global_sp_dir;
        pixel.reservoir_a.data.sample_incident_radiance =
            sample_li * sample_absdot;

        const real p = vp_dir_pdf * sp_dir_pdf;
        const real p_hat =
            first_shader.bsdf->eval(
                pixel.visible_wr, global_vp_dir, TransMode::Radiance).lum() *
            std::abs(cos(pixel.visible_normal, global_vp_dir)) *
            sp_f.lum() *
            pixel.reservoir_a.data.sample_incident_radiance.lum();

        pixel.reservoir_a.M    = 1;
        pixel.reservoir_a.wsum = p_hat / p;
        pixel.reservoir_a.W    = 1 / p; // 1 / (p_hat * M) * wsum
    }

    void create_initial_samples(
        const Scene    &scene,
        Image2D<Pixel> &pixels,
        ThreadPool     &threads,
        NativeSampler  *perthread_samplers,
        Arena          *perthread_arenas) const
    {
        const Vec2 inv_res = {
            real(1) / pixels.width(),
            real(1) / pixels.height()
        };

        auto thread_func = [&](int thread_idx, int y)
        {
            Arena temp_arena;

            auto &sampler = perthread_samplers[thread_idx];
            auto &arena   = perthread_arenas[thread_idx];

            auto *pixel_scanline = &pixels(y, 0);
            for(int x = 0; x < pixels.width(); ++x)
            {
                create_initial_sample_for_pixel(
                    inv_res, scene, { x, y }, pixel_scanline[x],
                    sampler, arena, temp_arena);

                if(temp_arena.used_bytes() >= 4 * 1024 * 1024)
                    temp_arena.release();

                if(stop_rendering_)
                    return;
            }
        };

        parallel_forrange(
            0, pixels.height(), thread_func, threads, params_.worker_count);
    }

    template<bool ResolveReservoirA>
    void resolve_gi_for_pixel(const Scene &scene, Pixel &pixel) const
    {
        if(!pixel.visible_bsdf)
            return;

        const auto &reservoir =
            ResolveReservoirA ? pixel.reservoir_a : pixel.reservoir_b;
        if(!reservoir.data.sample_bsdf)
            return;

        const auto f = eval<true>(scene, pixel, reservoir.data);
        if(f.is_finite() && std::isfinite(reservoir.W))
            pixel.accu_value += f * reservoir.W;
    }

    template<bool ResolveReservoirA>
    void resolve_gi(
        const Scene &scene, ThreadPool &threads, Image2D<Pixel> &pixels) const
    {
        auto thread_func = [&](int thread_idx, int y)
        {
            auto *pixel_scanline = &pixels(y, 0);
            for(int x = 0; x < pixels.width(); ++x)
            {
                resolve_gi_for_pixel<ResolveReservoirA>(
                    scene, pixel_scanline[x]);

                if(stop_rendering_)
                    return;
            }
        };

        thread::parallel_forrange(
            0, pixels.height(), thread_func, threads, params_.worker_count);
    }

    template<bool A2B>
    void reuse_spatial_for_pixel(
        const Scene    &scene,
        const Vec2i    &pixel_coord,
        Image2D<Pixel> &pixels,
        NativeSampler  &sampler) const
    {
        const int x_beg = (std::max)(
            0, pixel_coord.x - params_.spatial_reuse_radius);
        const int x_end = (std::min)(
            pixels.width(), pixel_coord.x + params_.spatial_reuse_radius + 1);

        const int y_beg = (std::max)(
            0, pixel_coord.y - params_.spatial_reuse_radius);
        const int y_end = (std::min)(
            pixels.height(), pixel_coord.y + params_.spatial_reuse_radius + 1);

        auto &pixel = pixels(pixel_coord);
        auto &output = A2B ? pixel.reservoir_b : pixel.reservoir_a;
        output.clear();
        output.data.sample_bsdf = nullptr;
        if(!pixel.visible_bsdf)
            return;

        static thread_local std::vector<Vec2i> nei_coords;
        nei_coords.clear();

        auto &input = A2B ? pixel.reservoir_a : pixel.reservoir_b;
        if(input.data.sample_bsdf)
            nei_coords.push_back(pixel_coord);

        for(int i = 0; i < params_.spatial_reuse_count; ++i)
        {
            const int sam_x = math::distribution::uniform_integer(
                x_beg, x_end, sampler.sample1().u);
            const int sam_y = math::distribution::uniform_integer(
                y_beg, y_end, sampler.sample1().u);

            auto &nei_pixel = pixels(sam_y, sam_x);
            if(!nei_pixel.visible_bsdf)
                continue;

            if(dot(nei_pixel.visible_normal, pixel.visible_normal) < real(0.8))
                continue;

            const real d1 = pixel.visible_dist;
            const real d2 = pixel.visible_dist;
            if(abs(d1 - d2) / (std::max)(d1, d2) >= real(0.05))
                continue;

            auto &nei_res = A2B ? nei_pixel.reservoir_a : nei_pixel.reservoir_b;
            if(!nei_res.data.sample_bsdf)
                continue;

            if(distance2(nei_pixel.visible_point, nei_res.data.sample_point) < EPS())
                continue;

            nei_coords.push_back({ sam_x, sam_y });
        }

        real selected_p_hat = 0;
        int sum_M = 0;
        for(auto &nei_coord : nei_coords)
        {
            auto &nei_pixel = pixels(nei_coord);
            auto &nei_res = A2B ? nei_pixel.reservoir_a : nei_pixel.reservoir_b;

            const real dist2U =
                distance2(nei_pixel.visible_point, nei_res.data.sample_point);
            const real JU =
                cos(pixel.visible_point - nei_res.data.sample_point,
                    nei_res.data.sample_normal) * dist2U;
            const real JD =
                cos(nei_pixel.visible_point - nei_res.data.sample_point,
                    nei_res.data.sample_normal) *
                distance2(pixel.visible_point, nei_res.data.sample_point);
            
            const real J = std::abs(JU / JD);
            const real p_hat = target_pdf<true>(scene, pixel, nei_res.data);

            if(output.update(
                nei_res.data,
                p_hat / J * nei_res.W * nei_res.M,
                sampler.sample1().u))
                selected_p_hat = p_hat / J;

            sum_M += nei_res.M;
        }

        if(!output.data.sample_bsdf)
            return;

        output.M = sum_M;

        if(selected_p_hat < EPS())
            output.W = 0;
        else
        {
            const real m = real(1) / output.M;
            output.W = m * output.wsum / selected_p_hat;
        }
    }

    template<bool A2B>
    void reuse_spatial(
        const Scene    &scene,
        ThreadPool     &threads,
        Image2D<Pixel> &pixels,
        NativeSampler  *thread_samplers) const
    {
        auto thread_func = [&](int thread_idx, int y)
        {
            auto &sampler = thread_samplers[thread_idx];
            for(int x = 0; x < pixels.width(); ++x)
            {
                if(stop_rendering_)
                    break;

                reuse_spatial_for_pixel<A2B>(
                    scene, { x, y }, pixels, sampler);
            }
        };

        parallel_forrange(
            0, pixels.height(), thread_func, threads, params_.worker_count);
    }

    ReSTIRGIParams params_;

    render::TraceParams trace_params_;

public:

    explicit ReSTIRGIRenderer(const ReSTIRGIParams &params)
        : params_(params)
    {
        params_.worker_count = thread::actual_worker_count(params_.worker_count);

        trace_params_.direct_illum_sample_count = 1;
        trace_params_.min_depth                 = params_.min_depth;
        trace_params_.max_depth                 = params_.max_depth;
        trace_params_.cont_prob                 = params_.cont_prob;
        trace_params_.specular_depth            = params_.specular_depth;
    }

    RenderTarget render(
        FilmFilterApplier   filter,
        Scene              &scene,
        RendererInteractor &reporter) override
    {
        const int w = filter.width(), h = filter.height();

        Image2D<Pixel> pixels(h, w);

        std::vector<NativeSampler> thread_samplers;
        std::vector<Arena>         thread_arenas(params_.worker_count);

        thread_samplers.reserve(params_.worker_count);
        for(int i = 0; i < params_.worker_count; ++i)
            thread_samplers.push_back(NativeSampler(i, false));

        ThreadPool threads;

        reporter.begin();
        reporter.new_stage();

        auto get_img = [&]
        {
            return pixels.map([](const Pixel &p)
            {
                const real w = p.accu_weight;
                const real r = w > 0 ? 1 / w : real(0);
                return r * p.accu_value;
            });
        };

        for(int i = 0; i < params_.spp; ++i)
        {
            for(auto &a : thread_arenas)
                a.release();

            create_initial_samples(
                scene, pixels, threads,
                thread_samplers.data(), thread_arenas.data());

            for(int j = 0; j < params_.I; ++j)
            {
                if((j & 1) == 0)
                {
                    reuse_spatial<true>(
                        scene, threads, pixels, thread_samplers.data());
                }
                else
                {
                    reuse_spatial<false>(
                        scene, threads, pixels, thread_samplers.data());
                }
            }

            if(stop_rendering_)
                break;

            if((params_.I & 1) == 0)
                resolve_gi<true>(scene, threads, pixels);
            else
                resolve_gi<false>(scene, threads, pixels);

            reporter.progress(100.0 * (i + 1) / params_.spp, get_img);

            if(stop_rendering_)
                break;
        }

        reporter.end_stage();
        reporter.end();

        RenderTarget render_target;
        render_target.image  .initialize(h, w);
        render_target.albedo .initialize(h, w);
        render_target.normal .initialize(h, w);
        render_target.denoise.initialize(h, w);

        for(int y = 0; y < h; ++y)
        {
            for(int x = 0; x < w; ++x)
            {
                auto &p = pixels(y, x);
                if(p.accu_weight <= real(0))
                    continue;

                const real r = 1 / p.accu_weight;
                render_target.image  (y, x) = r * p.accu_value;
                render_target.albedo (y, x) = r * p.accu_albedo;
                render_target.normal (y, x) = r * p.accu_normal;
                render_target.denoise(y, x) = r * p.accu_denoise;
            }
        }

        return render_target;
    }
};

RC<Renderer> create_restir_gi_renderer(const ReSTIRGIParams &params)
{
    return newRC<ReSTIRGIRenderer>(params);
}

AGZ_TRACER_END
