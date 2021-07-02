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
#include <agz-utils/thread.h>

AGZ_TRACER_BEGIN

namespace
{

    template<typename T>
    struct Reservoir
    {
        T data;
        
        real wsum = 0;
        real W    = 0;
        int  M    = 0;

        void update(const T &new_data, real w, real rnd)
        {
            wsum += w;
            if(w > 0 && rnd <= w / wsum)
                data = new_data;
            ++M;
        }

        void clear()
        {
            wsum = 0;
            W    = 0;
            M    = 0;
        }
    };

    struct ReservoirData
    {
        real ideal_pdf = 0;

        Vec3 light_pos_or_wi;
        Vec3 light_nor;
        Vec2 light_uv;

        const Light *light = nullptr;
        
        Vec2i pixel_coord;
    };

} // namespace anonymous

class ReSTIRRenderer : public Renderer
{
    struct Pixel
    {
        // accumulated data

        Spectrum value;
        float    weight = 0;
        Spectrum albedo;
        Vec3     normal;
        real     denoise;

        // per-frame data

        const BSDF *bsdf = nullptr;
        Vec3        visible_pos;
        Vec3        curr_normal;
        Vec3        wr;
    };

    using ImageBuffer     = Image2D<Pixel>;
    using ImageReservoirs = Image2D<Reservoir<ReservoirData>>;

    using ThreadPool = thread::thread_group_t;

    void create_pixel_reservoir(
        const Vec2i     &pixel_coord,
        ImageBuffer     &image_buffer,
        ImageReservoirs &image_reservoirs,
        const Scene     &scene,
        NativeSampler   &sampler,
        Arena           &general_arena,
        Arena           &bsdf_arena) const
    {
        auto &pixel     = image_buffer(pixel_coord.y, pixel_coord.x);
        auto &reservoir = image_reservoirs(pixel_coord.y, pixel_coord.x);

        pixel.bsdf = nullptr;

        reservoir.clear();
        reservoir.data.pixel_coord = pixel_coord;

        // generate ray

        const Vec2 film_coord =
        {
            (pixel_coord.x + sampler.sample1().u) / image_reservoirs.width(),
            (pixel_coord.y + sampler.sample1().u) / image_reservoirs.height(),
        };
        const auto cam_we =
            scene.get_camera()->sample_we(film_coord, sampler.sample2());
        const Ray ray(cam_we.pos_on_cam, cam_we.pos_to_out);

        // find first intersection

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
        {
            // fill image buffer

            Spectrum rad;
            if(auto light = scene.envir_light())
                rad = light->radiance(ray.o, ray.d);
            rad *= cam_we.throughput;

            if(rad.is_finite())
            {
                pixel.value   += rad;
                pixel.weight  += 1;
                pixel.denoise += 1;
            }

            return;
        }

        if(auto light = inct.entity->as_light())
        {
            pixel.value += light->radiance(
                inct.pos, inct.geometry_coord.z, inct.uv, inct.wr);
        }

        const auto shading_point = inct.material->shade(inct, bsdf_arena);

        // fill gbuffer

        const real denoise = inct.entity->get_no_denoise_flag() ? 0.0f : 1.0f;

        pixel.weight  += 1;
        pixel.denoise += denoise;
        pixel.albedo  += shading_point.bsdf->albedo();
        pixel.normal  += shading_point.shading_normal;

        pixel.bsdf        = shading_point.bsdf;
        pixel.visible_pos = inct.pos;
        pixel.curr_normal = inct.geometry_coord.z;
        pixel.wr          = inct.wr;

        // wrs candidates

        for(int i = 0; i < params_.M; ++i)
        {
            const auto [light, select_light_pdf] =
                scene.sample_light(sampler.sample1());
            const auto light_sample =
                light->sample(inct.pos, sampler.sample5());
            if(!light_sample.valid())
                continue;
            const FVec3 wi = light_sample.ref_to_light();

            // compute actual/ideal pdf

            const real p = select_light_pdf * light_sample.pdf;
            
            const FSpectrum bsdf = shading_point.bsdf->eval_all(
                wi, inct.wr, TransMode::Radiance);

            const real absdot = std::abs(cos(wi, inct.geometry_coord.z));

            const real p_hat =
                bsdf.lum() * absdot * light_sample.radiance.lum();

            // update resevoir

            reservoir.update(
                {
                    p_hat,
                    light->is_area() ? light_sample.pos : wi,
                    light_sample.nor,
                    light_sample.uv,
                    light,
                    pixel_coord
                },
                p_hat / p, sampler.sample1().u);
        }

        reservoir.M = params_.M;

        // test visibility and compute corr factor

        if(reservoir.wsum)
        {
            if(!test_visibility(scene, inct.pos, reservoir.data))
                reservoir.W = 0;
            else
            {
                reservoir.W =
                    reservoir.wsum / (reservoir.data.ideal_pdf * reservoir.M);
            }
        }
    }

    bool test_visibility(
        const Scene &scene, const Vec3 &pos, const ReservoirData &red) const
    {
        if(red.light->is_area())
            return scene.visible(pos, red.light_pos_or_wi);
        return !scene.has_intersection(Ray(pos, red.light_pos_or_wi, EPS()));
    }

    real compute_ideal_pdf(const Pixel &pixel, const ReservoirData &data) const
    {
        const FVec3 wi = data.light->is_area() ?
            (data.light_pos_or_wi - pixel.visible_pos) : data.light_pos_or_wi;

        const Spectrum bsdf =
            pixel.bsdf->eval_all(wi, pixel.wr, TransMode::Radiance);

        const real abscos = std::abs(cos(wi, pixel.curr_normal));

        const Spectrum le =
            data.light->is_area() ?
            data.light->as_area()->radiance(
                data.light_pos_or_wi,
                data.light_nor,
                data.light_uv,
                -wi) :
            data.light->as_envir()->radiance(
                pixel.visible_pos, wi);

        return bsdf.lum() * abscos * le.lum();
    }

    void create_frame_reservoirs(
        ThreadPool         &threads,
        ImageBuffer        &image_buffer,
        ImageReservoirs    &image_reservoirs,
        const Scene        &scene,
        NativeSampler      *thread_samplers,
        Arena              *thread_bsdf_arena) const
    {
        auto thread_func = [&](int thread_idx, int y)
        {
            NativeSampler &sampler    = thread_samplers[thread_idx];
            Arena         &bsdf_arena = thread_bsdf_arena[thread_idx];

            Arena general_arena;

            for(int x = 0; x < image_reservoirs.width(); ++x)
            {
                create_pixel_reservoir(
                    { x, y }, image_buffer, image_reservoirs, scene,
                    sampler, general_arena, bsdf_arena);

                if(general_arena.used_bytes() > 4 * 1024 * 1024)
                    general_arena.release();

                if(stop_rendering_)
                    return;
            }
        };

        parallel_forrange(
            0, image_reservoirs.height(),
            thread_func, threads, params_.worker_count);
    }

    void combine_neghbor_reservoirs(
        const Scene           &scene,
        const Vec2i           &pixel_coord,
        const ImageBuffer     &image_buffer,
        const ImageReservoirs &input_reservoirs,
        ImageReservoirs       &output_reservoirs,
        NativeSampler         &sampler)
    {
        const int x_beg = (std::max)(
            0, pixel_coord.x - params_.spatial_reuse_radius);
        const int x_end = (std::min)(
            input_reservoirs.width(),
            pixel_coord.x + params_.spatial_reuse_radius + 1);
        
        const int y_beg = (std::max)(
            0, pixel_coord.y - params_.spatial_reuse_radius);
        const int y_end = (std::min)(
            input_reservoirs.height(),
            pixel_coord.y + params_.spatial_reuse_radius + 1);

        auto &input  = input_reservoirs (pixel_coord.y, pixel_coord.x);
        auto &output = output_reservoirs(pixel_coord.y, pixel_coord.x);

        output.clear();

        auto &pixel = image_buffer(pixel_coord.y, pixel_coord.x);
        if(!pixel.bsdf)
            return;

        static thread_local std::vector<Vec2i> nei_coords;
        
        nei_coords.clear();
        if(input.wsum)
            nei_coords.push_back(pixel_coord);

        for(int i = 0; i < params_.spatial_reuse_count; ++i)
        {
            const int sam_x = math::distribution::uniform_integer(
                x_beg, x_end, sampler.sample1().u);
            const int sam_y = math::distribution::uniform_integer(
                y_beg, y_end, sampler.sample1().u);

            auto &nei_pixel = image_buffer(sam_y, sam_x);
            if(!nei_pixel.bsdf)
                continue;

            if(dot(nei_pixel.curr_normal, pixel.curr_normal) < 0.93f)
                continue;

            auto &nei_reservoir = input_reservoirs(sam_y, sam_x);
            if(nei_reservoir.wsum)
                nei_coords.push_back({ sam_x, sam_y });
        }

        for(auto &nei_coord : nei_coords)
        {
            auto &nei_reservoir = input_reservoirs(nei_coord.y, nei_coord.x);
            const real p_hat = compute_ideal_pdf(pixel, nei_reservoir.data);
            
            auto data = nei_reservoir.data;
            data.ideal_pdf = p_hat;

            output.update(
                data,
                p_hat * nei_reservoir.W * nei_reservoir.M,
                sampler.sample1().u);
        }

        if(!output.wsum)
            return;

        output.M = 0;
        for(auto &nei_coord : nei_coords)
        {
            auto &nei_reservoir = input_reservoirs(nei_coord.y, nei_coord.x);
            output.M += nei_reservoir.M;
        }

        int Z = 0;
        for(auto &nei_coord : nei_coords)
        {
            auto &nei_reservoir = input_reservoirs(nei_coord.y, nei_coord.x);
            auto &nei_pixel     = image_buffer(nei_coord.y, nei_coord.x);

            if(!test_visibility(scene, nei_pixel.visible_pos, output.data))
                continue;

            const real p_hat = compute_ideal_pdf(nei_pixel, output.data);
            if(p_hat <= 0)
                continue;

            Z += nei_reservoir.M;
        }

        const real m = Z > 0 ? real(1) / Z : real(0);
        output.W = m * output.wsum / output.data.ideal_pdf;
    }

    void reuse_spatial(
        const Scene       &scene,
        ThreadPool        &threads,
        const ImageBuffer &image_buffer,
        ImageReservoirs   &input_reservoirs,
        ImageReservoirs   &output_reservoirs,
        NativeSampler     *thread_samplers)
    {
        auto thread_func = [&](int thread_idx, int y)
        {
            auto &sampler = thread_samplers[thread_idx];
            for(int x = 0; x < input_reservoirs.width(); ++x)
            {
                combine_neghbor_reservoirs(
                    scene, { x, y }, image_buffer,
                    input_reservoirs, output_reservoirs, sampler);
            }
        };

        parallel_forrange(
            0, input_reservoirs.height(),
            thread_func, threads, params_.worker_count);
    }

    FSpectrum resolve_pixel(
        const Scene                    &scene,
        const Pixel                    &pixel,
        const Reservoir<ReservoirData> &reservoir)
    {
        if(!reservoir.wsum)
            return {};
        assert(pixel.bsdf);

        if(!test_visibility(scene, pixel.visible_pos, reservoir.data))
            return {};

        const FVec3 wi = reservoir.data.light->is_area() ?
            (reservoir.data.light_pos_or_wi - pixel.visible_pos) :
            reservoir.data.light_pos_or_wi;

        const FSpectrum bsdf =
            pixel.bsdf->eval_all(wi, pixel.wr, TransMode::Radiance);

        const real absdot = abs(cos(pixel.curr_normal, wi));

        FSpectrum le;
        if(auto env = reservoir.data.light->as_envir())
        {
            le = env->radiance(pixel.visible_pos, wi);
        }
        else
        {
            le = reservoir.data.light->as_area()->radiance(
                reservoir.data.light_pos_or_wi,
                reservoir.data.light_nor,
                reservoir.data.light_uv,
                -wi);
        }

        return bsdf * absdot * le * reservoir.W;
    }

    ReSTIRParams params_;

public:

    explicit ReSTIRRenderer(ReSTIRParams params)
    {
        params_ = params;
        params_.worker_count = thread::actual_worker_count(params_.worker_count);
    }

    RenderTarget render(
        FilmFilterApplier   filter,
        Scene              &scene,
        RendererInteractor &reporter) override
    {
        ImageBuffer     image_buffer      (filter.height(), filter.width());
        ImageReservoirs image_reservoirs_a(filter.height(), filter.width());
        ImageReservoirs image_reservoirs_b(filter.height(), filter.width());

        std::vector<NativeSampler> thread_samplers;
        std::vector<Arena>         thread_bsdf_arenas(params_.worker_count);

        thread_samplers.reserve(params_.worker_count);
        for(int i = 0; i < params_.worker_count; ++i)
            thread_samplers.push_back(NativeSampler(i, false));

        reporter.begin();
        reporter.new_stage();

        ThreadPool threads;

        auto get_img = [&]
        {
            return image_buffer.map([](const Pixel &p)
            {
                const real w = p.weight;
                const real ratio = w > 0 ? 1 / w : real(1);
                return ratio * p.value;
            });
        };
        
        for(int i = 0; i < params_.spp; ++i)
        {
            for(auto &a : thread_bsdf_arenas)
                a.release();

            create_frame_reservoirs(
                threads, image_buffer, image_reservoirs_a, scene,
                thread_samplers.data(), thread_bsdf_arenas.data());

            if(stop_rendering_)
                break;

            auto src_reservoirs = &image_reservoirs_a;
            auto dst_reservoirs = &image_reservoirs_b;
            //for(int j = 0; j < params_.I; ++j)
            //{
                reuse_spatial(
                    scene, threads, image_buffer,
                    *src_reservoirs, *dst_reservoirs,
                    thread_samplers.data());
                std::swap(src_reservoirs, dst_reservoirs);
            //}
            auto &image_reservoirs = *src_reservoirs;

            auto resolve_thread_func = [&](int thread_idx, int y)
            {
                for(int x = 0; x < filter.width(); ++x)
                {
                    auto &reservoir = image_reservoirs(y, x);
                    auto &pixel     = image_buffer(y, x);

                    const FSpectrum value = resolve_pixel(scene, pixel, reservoir);
                    if(value.is_finite())
                        image_buffer(y, x).value += value;

                    if(stop_rendering_)
                        break;
                }
            };
            parallel_forrange(
                0, filter.height(),
                resolve_thread_func, threads, params_.worker_count);

            reporter.progress(100.0 * (i + 1) / params_.spp, get_img);

            if(stop_rendering_)
                break;
        }

        reporter.end_stage();
        reporter.end();

        RenderTarget render_target;
        render_target.image = get_img();
        
        return render_target;
    }
};

RC<Renderer> create_restir_renderer(const ReSTIRParams &params)
{
    return newRC<ReSTIRRenderer>(params);
}

AGZ_TRACER_END
