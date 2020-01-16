#include <atomic>
#include <mutex>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/reporter.h>
#include <agz/tracer/core/renderer.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/factory/raw/renderer.h>
#include <agz/utility/thread.h>

AGZ_TRACER_BEGIN

class BDPTRenderer : public Renderer
{
public:

    explicit BDPTRenderer(const BDPTRendererParams &params);

    RenderTarget render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter) override;

private:

    using FilmGrid     = FilmFilterApplier::FilmGrid<Spectrum, real, Spectrum, Vec3, real>;
    using ParticleFilm = FilmFilterApplier::FilmGrid<Spectrum>;

    struct GPixel
    {
        Spectrum albedo;
        Vec3 normal;
        real denoise = 1;
    };

    /*
     * @brief 路径中的顶点
     *
     * [e]表示仅在entity不为空时有意义
     */
    struct Vertex
    {
        // 顶点位置，entity为空时表示ray.d
        Vec3 pos;

        // 顶点法线[e]
        Vec3 nor;

        // 从起点到该顶点的前一个顶点间累计的bsdf
        // 包括摄像机we和光源rad
        Spectrum accu_bsdf;

        // 从起点到该顶点的累计proj pdf
        // 包括最开始的pdf_pos
        real accu_proj_pdf = 0;

        // 沿摄像机->光源方向采样到该顶点的proj pdf
        // 摄像机起点的该值表示摄像机的pdf_pos
        real pdf_fwd = 0;

        // 沿光源->摄像机方向采样到该顶点的proj pdf
        // 光源起点的该值表示光源点的pdf_pos
        real pdf_bwd = 0;

        // 同上一个顶点间的G[e]
        real G_with_last = 0;

        // 交点entity，为空时表示是摄像机子路经末端，没有交点
        const Entity *entity = nullptr;

        // 交点bsdf[e]
        const BSDF *bsdf = nullptr;

        bool is_delta = false;

        bool is_entity() const noexcept
        {
            return entity != nullptr;
        }
    };

    struct GridParams
    {
        const Scene  &scene;
        Sampler      &sampler;
        FilmGrid     &film_grid;
        ParticleFilm &particle_film;
        const Vec2i  &full_res;
        Vertex       *cam_subpath_space = nullptr;
        Vertex       *lht_subpath_space = nullptr;
    };

    struct CameraSubpath
    {
        real pixel_x    = 0;
        real pixel_y    = 0;

        int vtx_cnt     = 0;
        Vertex *subpath = nullptr;

        const CameraSampleWeResult sam;
        const CameraWePDFResult    pdf;

        GPixel gpixel;
    };

    struct LightSubpath
    {
        int     vtx_cnt = 0;
        Vertex *subpath = nullptr;

        real select_light_pdf = 0;
        const Light *light    = nullptr;
        const LightEmitResult emit;
    };

    struct ConnectedPath
    {
        const Scene &scene;
        const Light  *light;
        const Camera *camera;

        real select_light_pdf = 0;
        const LightEmitResult emit;

        Vertex *cam_subpath; int s;
        Vertex *lht_subpath; int t;

        Sampler &sampler;

        ParticleFilm &particle_film;
        const Vec2i full_res;
    };

    using TmpAssign = misc::scope_assignment_t<real>;

    static real G(const Vec3 &a, const Vec3 &b, const Vec3 &na, const Vec3 &nb) noexcept;

    static real z2o(real x) noexcept;

    static real n2o(real x) noexcept;

    int render_grid(
        const Scene &scene, Sampler &sampler, FilmGrid &film_grid, ParticleFilm &particle_film, const Vec2i &full_res);

    void eval_sample(int px, int py, const GridParams &grid_params, Arena &arena);

    CameraSubpath build_camera_subpath(
        int px, int py, const Scene &scene, const Vec2i &full_res,
        Sampler &sampler, Arena &arena, Vertex *subpath_space) const;

    LightSubpath build_light_subpath(
        const Scene &scene, Sampler &sampler, Arena &arena, Vertex *subpath_space) const;

    Spectrum eval_connected_subpath(const ConnectedPath &connected_path) const;

    Spectrum contrib_s2_t0_path(const ConnectedPath &connected_path) const;

    Spectrum contrib_s1_tx_path(const ConnectedPath &connected_path) const;

    Spectrum contrib_sx_t0_path(const ConnectedPath &connected_path) const;

    Spectrum contrib_sx_t1_path(const ConnectedPath &connected_path) const;

    Spectrum contrib_sx_tx_path(const ConnectedPath &connected_path) const;

    /**
     * @brief 计算给定路径的mis weight
     *
     * 保证connected_path是一条包含至少3个顶点的合法路径
     *
     * connected_path中所有顶点的pdf_bwd、pdf_fwd和is_delta都必须被完整填充，其他诸如pos、nor都不重要
     */
    real weight_common(const ConnectedPath &connected_path, real G_between_subpaths) const;

    real weight_s1_tx_path(const ConnectedPath &connected_path) const;

    real weight_sx_t0_path(const ConnectedPath &connected_path) const;

    real weight_sx_t1_path(const ConnectedPath &connected_path, const LightSampleResult &lht_sam_wi) const;

    real weight_sx_tx_path(const ConnectedPath &connected_path) const;

    BDPTRendererParams params_;
};

BDPTRenderer::BDPTRenderer(const BDPTRendererParams &params)
    : params_(params)
{
    
}

RenderTarget BDPTRenderer::render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter)
{
    scene.start_rendering();
    reporter.begin();
    reporter.new_stage();

    const int width = filter.width();
    const int height = filter.height();
    const int x_task_count = (width + params_.task_grid_size - 1) / params_.task_grid_size;
    const int y_task_count = (height + params_.task_grid_size - 1) / params_.task_grid_size;
    const int total_task_count = x_task_count * y_task_count;

    std::mutex reporter_mutex;
    std::atomic<int> next_task_id = 0;
    std::atomic<int> total_particle_count = 0;

    ImageBufferTemplate<true, true, true, true, true> image_buffer(width, height);

    auto render_func = [
        &filter,
        &image_buffer,
        &scene,
        &reporter,
        &reporter_mutex,
        &next_task_id,
        &total_particle_count,
        x_task_count,
        total_task_count,
        task_grid_size = params_.task_grid_size,
        this
    ] (Sampler *sampler, Image2D<Spectrum> *particle_image)
    {
        const Vec2i full_res = { filter.width(), filter.height() };
        ParticleFilm particle_film = filter.bind_to(
            { { 0, 0 }, { filter.width(), filter.height() } }, *particle_image);

        for(;;)
        {
            const int task_id = next_task_id++;
            if(task_id >= total_task_count)
                break;

            const int grid_y_index = task_id / x_task_count;
            const int grid_x_index = task_id % x_task_count;
            
            const int x_beg = grid_x_index * task_grid_size;
            const int y_beg = grid_y_index * task_grid_size;
            const int x_end = (std::min)(x_beg + task_grid_size, filter.width());
            const int y_end = (std::min)(y_beg + task_grid_size, filter.height());

            FilmGrid film_grid = filter.bind_to(
                { { x_beg, y_beg }, { x_end - 1, y_end - 1 } },
                image_buffer.value, image_buffer.weight,
                image_buffer.albedo, image_buffer.normal, image_buffer.denoise);

            total_particle_count += this->render_grid(scene, *sampler, film_grid, particle_film, full_res);

            const real percent = real(100) * (task_id + 1) / total_task_count;
            std::lock_guard lk(reporter_mutex);
            reporter.progress(percent);
        }
    };

    // 创建工作线程

    const int worker_count = thread::actual_worker_count(params_.worker_count);
    Arena sampler_arena;

    std::vector<std::thread> threads;
    std::vector<Image2D<Spectrum>> particle_images;
    threads.reserve(worker_count);
    particle_images.resize(worker_count, Image2D<Spectrum>(height, width));

    for(int i = 0; i < worker_count; ++i)
    {
        Sampler *sampler = params_.sampler_prototype->clone(i, sampler_arena);
        threads.emplace_back(render_func, sampler, &particle_images[i]);
    }

    for(auto &t : threads)
        t.join();

    // 构建forward tracing结果

    const Image2D<real> image_buffer_ratio = image_buffer.weight.map([](real w)
    {
        return w > 0 ? 1 / w : real(1);
    });

    RenderTarget render_target;
    render_target.image   = image_buffer_ratio * image_buffer.value;
    render_target.albedo  = image_buffer_ratio * image_buffer.albedo;
    render_target.normal  = image_buffer_ratio * image_buffer.normal;
    render_target.denoise = image_buffer_ratio * image_buffer.denoise;

    // 合并particle images

    const real particle_scale = static_cast<real>(width * height) / (std::max)(1, total_particle_count.load());
    for(auto &pi : particle_images)
        render_target.image = render_target.image + particle_scale * pi;

    reporter.end_stage();
    reporter.end();

    return render_target;
}

real BDPTRenderer::G(const Vec3 &a, const Vec3 &b, const Vec3 &na, const Vec3 &nb) noexcept
{
    const Vec3 d = a - b;
    return std::abs(cos(d, na) * cos(d, nb)) / d.length_square();
}

real BDPTRenderer::z2o(real x) noexcept
{
    return !std::isnan(x) && x > EPS ? x : real(1);
}

real BDPTRenderer::n2o(real x) noexcept
{
    return std::isnan(x) ? 1 : x;
}

int BDPTRenderer::render_grid(const Scene &scene, Sampler &sampler, FilmGrid &film_grid, ParticleFilm &particle_film, const Vec2i &full_res)
{
    if(scene.lights().empty())
        return 0;

    const Rect2i sample_pixels = film_grid.sample_pixels();
    
    Arena arena;
    std::vector<Vertex> cam_subpath_space(params_.cam_max_vtx_cnt);
    std::vector<Vertex> lht_subpath_space(params_.lht_max_vtx_cnt);

    GridParams grid_params = {
        scene, sampler, film_grid, particle_film, full_res,
        cam_subpath_space.data(), lht_subpath_space.data()
    };

    int particle_count = 0;
    for(int py = sample_pixels.low.y; py <= sample_pixels.high.y; ++py)
    {
        for(int px = sample_pixels.low.x; px <= sample_pixels.high.x; ++px)
        {
            sampler.start_pixel(px, py);
            do
            {
                ++particle_count;
                eval_sample(px, py, grid_params, arena);

            } while(sampler.next_sample());
        }
    }

    return particle_count;
}

void BDPTRenderer::eval_sample(int px, int py, const GridParams &grid_params, Arena &arena)
{
    const CameraSubpath cam_subpath = build_camera_subpath(
        px, py, grid_params.scene, grid_params.full_res, grid_params.sampler, arena, grid_params.cam_subpath_space);

    const LightSubpath lht_subpath = build_light_subpath(
        grid_params.scene, grid_params.sampler, arena, grid_params.lht_subpath_space);

    ConnectedPath connected_path = {
        grid_params.scene, lht_subpath.light, grid_params.scene.get_camera(),
        lht_subpath.select_light_pdf, lht_subpath.emit,
        cam_subpath.subpath, 0, lht_subpath.subpath, 0,
        grid_params.sampler, grid_params.particle_film, grid_params.full_res
    };

    Spectrum radiance;
    for(int s = 1; s <= cam_subpath.vtx_cnt; ++s)
    {
        for(int t = 0; t <= lht_subpath.vtx_cnt; ++t)
        {
            connected_path.s = s;
            connected_path.t = t;
            radiance += eval_connected_subpath(connected_path);
        }
    }

    grid_params.film_grid.apply(
        cam_subpath.pixel_x, cam_subpath.pixel_y,
        radiance, 1,
        cam_subpath.gpixel.albedo,
        cam_subpath.gpixel.normal,
        cam_subpath.gpixel.denoise);
}

BDPTRenderer::CameraSubpath BDPTRenderer::build_camera_subpath(
    int px, int py, const Scene &scene, const Vec2i &full_res,
    Sampler &sampler, Arena &arena, Vertex *subpath_space) const
{
    assert(subpath_space);

    // sample camera ray

    const Camera *camera = scene.get_camera();
    const Sample2 film_sam = sampler.sample2();

    const Vec2 pixel_coord = {
        px + film_sam.u,
        py + film_sam.v
    };

    const Vec2 film_coord = {
        pixel_coord.x / full_res.x,
        pixel_coord.y / full_res.y
    };

    const auto cam_sam = camera->sample_we(film_coord, sampler.sample2());
    const auto cam_pdf = camera->pdf_we(cam_sam.pos_on_cam, cam_sam.pos_to_out);

    // fill the first vertex

    subpath_space[0].pos           = cam_sam.pos_on_cam;
    subpath_space[0].nor           = cam_sam.nor_on_cam;
    subpath_space[0].accu_bsdf     = Spectrum(1);
    subpath_space[0].accu_proj_pdf = cam_pdf.pdf_pos;
    subpath_space[0].pdf_fwd       = cam_pdf.pdf_pos;
    subpath_space[0].pdf_bwd       = 0;
    subpath_space[0].G_with_last   = 1;

    // g-buffer pixel
    GPixel gpixel;

    // 从上一个顶点到这一顶点的proj pdf
    real proj_pdf = cam_pdf.pdf_dir / std::abs(cos(cam_sam.nor_on_cam, cam_sam.pos_to_out));

    // 累计proj pdf，包括最开始的pdf_pos
    real accu_proj_pdf = cam_pdf.pdf_pos * proj_pdf;

    // 累计bsdf，包括最开始的we
    Spectrum accu_bsdf = camera->eval_we(cam_sam.pos_on_cam, cam_sam.pos_to_out).we;

    // 上一顶点的位置
    Vec3 last_pos = cam_sam.pos_on_cam;

    // 上一顶点的法线
    Vec3 last_nor = cam_sam.nor_on_cam;

    // 当前射线
    Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

    int cam_vtx_cnt = 1;
    while(cam_vtx_cnt < params_.cam_max_vtx_cnt)
    {
        // 寻找最近顶点

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
        {
            // camera end vertex的entity为nullptr表示无交点

            Vertex end_vertex;
            end_vertex.pos           = ray.d;
            end_vertex.accu_bsdf     = accu_bsdf;
            end_vertex.accu_proj_pdf = accu_proj_pdf;
            end_vertex.pdf_fwd       = proj_pdf;
            subpath_space[cam_vtx_cnt++] = end_vertex;
            break;
        }

        const ShadingPoint shd = inct.material->shade(inct, arena);

        // 填充g-buffer pixel

        if(cam_vtx_cnt == 1)
        {
            gpixel.albedo  = shd.bsdf->albedo();
            gpixel.normal  = shd.shading_normal;
            gpixel.denoise = inct.entity->get_no_denoise_flag() ? real(0) : real(1);
        }

        // 添加新顶点

        Vertex vertex;
        vertex.pos           = inct.pos;
        vertex.nor           = inct.geometry_coord.z;
        vertex.accu_bsdf     = accu_bsdf;
        vertex.accu_proj_pdf = accu_proj_pdf;
        vertex.pdf_fwd       = proj_pdf;
        vertex.G_with_last   = G(inct.pos, last_pos, inct.geometry_coord.z, last_nor);
        vertex.entity        = inct.entity;
        vertex.bsdf          = shd.bsdf;
        vertex.is_delta      = shd.bsdf->is_delta();

        last_pos = vertex.pos;
        last_nor = vertex.nor;

        subpath_space[cam_vtx_cnt++] = vertex;

        // 采样bsdf

        const auto bsdf_sample = shd.bsdf->sample(inct.wr, TransportMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f)
            break;

        proj_pdf       = bsdf_sample.pdf / std::abs(cos(vertex.nor, bsdf_sample.dir));
        accu_bsdf     *= bsdf_sample.f;
        accu_proj_pdf *= proj_pdf;

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    // 填充backward proj pdf

    const int max_bwd_index = subpath_space[cam_vtx_cnt - 1].is_entity() ? cam_vtx_cnt - 3 : cam_vtx_cnt - 4;

    for(int i = 0; i <= max_bwd_index; ++i)
    {
        Vertex &a = subpath_space[i];
        const Vertex &b = subpath_space[i + 1];
        const Vertex &c = subpath_space[i + 2];

        // store pdf(c -> b -> a) into a.bwd_pdf

        const real pdf = b.bsdf->pdf(c.pos - b.pos, a.pos - b.pos, TransportMode::Importance);
        a.pdf_bwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    if(!subpath_space[cam_vtx_cnt - 1].is_entity() && cam_vtx_cnt >= 3)
    {
        // .., a, b, c

        Vertex &a = subpath_space[cam_vtx_cnt - 3];
        const Vertex &b = subpath_space[cam_vtx_cnt - 2];
        const Vertex &c = subpath_space[cam_vtx_cnt - 1];

        const real pdf = b.bsdf->pdf(c.pos, a.pos - b.pos, TransportMode::Importance);
        a.pdf_bwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    return CameraSubpath {
        pixel_coord.x, pixel_coord.y,
        cam_vtx_cnt, subpath_space,
        cam_sam, cam_pdf, gpixel
    };
}

BDPTRenderer::LightSubpath BDPTRenderer::build_light_subpath(
    const Scene &scene, Sampler &sampler, Arena &arena, Vertex *subpath_space) const
{
    assert(subpath_space);

    // sample light emission

    const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
    assert(light);
    const auto light_emit = light->emit(sampler.sample5());

    // fill the first vertex

    subpath_space[0].pos           = light_emit.pos;
    subpath_space[0].nor           = light_emit.nor;
    subpath_space[0].accu_bsdf     = Spectrum(1);
    subpath_space[0].accu_proj_pdf = select_light_pdf * light_emit.pdf_pos;
    subpath_space[0].pdf_fwd       = 0;
    subpath_space[0].pdf_bwd       = select_light_pdf * light_emit.pdf_pos;
    subpath_space[0].G_with_last   = 1;

    // 从上一个顶点到这一顶点的proj pdf
    real proj_pdf = light_emit.pdf_dir / std::abs(cos(light_emit.nor, light_emit.dir));

    // 累计proj pdf，包括pdf pos
    real accu_proj_pdf = select_light_pdf * light_emit.pdf_pos * proj_pdf;

    // 累计bsdf，包括light radiance
    Spectrum accu_bsdf = light_emit.radiance;

    // 上一顶点位置
    Vec3 last_pos = light_emit.pos;

    // 上一顶点法线
    Vec3 last_nor = light_emit.nor;

    // 当前射线
    Ray ray(light_emit.pos, light_emit.dir, EPS);

    int lht_vtx_cnt = 1;
    while(lht_vtx_cnt < params_.lht_max_vtx_cnt)
    {
        // 寻找最近顶点

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
            break;

        const ShadingPoint shd = inct.material->shade(inct, arena);

        // 添加新顶点

        Vertex vertex;
        vertex.pos           = inct.pos;
        vertex.nor           = inct.geometry_coord.z;
        vertex.accu_bsdf     = accu_bsdf;
        vertex.accu_proj_pdf = accu_proj_pdf;
        vertex.pdf_fwd       = 0;
        vertex.pdf_bwd       = proj_pdf;
        vertex.G_with_last   = G(inct.pos, last_pos, inct.geometry_coord.z, last_nor);
        vertex.entity        = inct.entity;
        vertex.bsdf          = shd.bsdf;
        vertex.is_delta      = shd.bsdf->is_delta();

        last_pos = vertex.pos;
        last_nor = vertex.nor;

        subpath_space[lht_vtx_cnt++] = vertex;

        // 采样bsdf

        const auto bsdf_sample = shd.bsdf->sample(inct.wr, TransportMode::Importance, sampler.sample3());
        if(!bsdf_sample.f)
            break;

        proj_pdf       = bsdf_sample.pdf / std::abs(cos(vertex.nor, bsdf_sample.dir));
        accu_bsdf     *= bsdf_sample.f;
        accu_proj_pdf *= proj_pdf;

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    // 填充forward proj pdf

    const int max_fwd_index = lht_vtx_cnt - 3;

    for(int i = 1; i <= max_fwd_index; ++i)
    {
        Vertex &a = subpath_space[i];
        const Vertex &b = subpath_space[i + 1];
        const Vertex &c = subpath_space[i + 2];

        // store pdf(c -> b -> a) into a.pdf_fwd

        const real pdf = b.bsdf->pdf(c.pos - b.pos, a.pos - b.pos, TransportMode::Radiance);
        a.pdf_fwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    // 填充subpath_space[0].pdf_fwd

    if(lht_vtx_cnt >= 3)
    {
        Vertex &a = subpath_space[0];
        const Vertex &b = subpath_space[1];
        const Vertex &c = subpath_space[2];

        const real pdf = b.bsdf->pdf(c.pos - b.pos, -light_emit.dir, TransportMode::Radiance);
        a.pdf_fwd = pdf / std::abs(cos(b.nor, light_emit.dir));
    }

    return LightSubpath{ lht_vtx_cnt, subpath_space, select_light_pdf, light, light_emit };
}

Spectrum BDPTRenderer::eval_connected_subpath(const ConnectedPath &connected_path) const
{
    // 至少两个顶点才能构成一条路径

    const int s = connected_path.s;
    const int t = connected_path.t;
    const int path_vtx_cnt = s + t;

    if(path_vtx_cnt < 2)
        return {};

    // 对2顶点路径，只使用unidirectional path tracing这一种采样技术

    if(path_vtx_cnt == 2)
        return s == 2 ? contrib_s2_t0_path(connected_path) : Spectrum();

    if(s == 1)
    {
        assert(t >= 2);
        return contrib_s1_tx_path(connected_path);
    }

    if(t == 0)
    {
        assert(s >= 3);
        return contrib_sx_t0_path(connected_path);
    }

    if(t == 1)
    {
        assert(s >= 2);
        return contrib_sx_t1_path(connected_path);
    }

    return contrib_sx_tx_path(connected_path);
}

Spectrum BDPTRenderer::contrib_s2_t0_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s == 2 && connected_path.t == 0);

    const Vertex &cam_beg = connected_path.cam_subpath[0];
    const Vertex &cam_end = connected_path.cam_subpath[1];

    if(cam_end.is_entity())
    {
        const AreaLight *light = cam_end.entity->as_light();
        if(!light)
            return {};

        const Spectrum radiance = light->radiance(cam_end.pos, cam_end.nor, cam_beg.pos - cam_end.pos);
        return radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
    }

    Spectrum radiance;
    if(auto light = connected_path.scene.envir_light())
        radiance = light->radiance(cam_beg.pos, cam_end.pos);

    return radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
}

Spectrum BDPTRenderer::contrib_s1_tx_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s == 1 && connected_path.t >= 2);

    const int t = connected_path.t;
    Vertex &cam_beg = connected_path.cam_subpath[0];
    const Vertex &lht_end = connected_path.lht_subpath[t - 1];
    const Vertex &lht_bend = connected_path.lht_subpath[t - 2];

    const CameraEvalWeResult cam_we = connected_path.camera->eval_we(cam_beg.pos, lht_end.pos - cam_beg.pos);
    if(!cam_we.we)
        return {};

    const Vec2 pixel_coord = {
        cam_we.film_coord.x * connected_path.full_res.x,
        cam_we.film_coord.y * connected_path.full_res.y
    };

    if(!connected_path.particle_film.in_sample_pixel_bound(pixel_coord.x, pixel_coord.y))
        return {};

    if(!connected_path.scene.visible(cam_beg.pos, lht_end.pos))
        return {};

    const Spectrum bsdf = lht_end.bsdf->eval(lht_bend.pos - lht_end.pos, cam_beg.pos - lht_end.pos, TransportMode::Radiance);
    if(!bsdf)
        return {};

    const real G_val = G(cam_beg.pos, lht_end.pos, cam_beg.nor, lht_end.nor);
    if(G_val < EPS)
        return {};

    const Spectrum contrib = cam_we.we * G_val * bsdf * lht_end.accu_bsdf / (cam_beg.accu_proj_pdf * lht_end.accu_proj_pdf);

    if(!contrib.is_black())
    {
        const real weight = weight_s1_tx_path(connected_path);
        connected_path.particle_film.apply(pixel_coord.x, pixel_coord.y, weight * contrib);
    }

    return {};
}

Spectrum BDPTRenderer::contrib_sx_t0_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s >= 3);
    assert(connected_path.t == 0);

    const int s = connected_path.s;
    const Vertex &cam_end = connected_path.cam_subpath[s - 1];
    const Vertex &cam_bend = connected_path.cam_subpath[s - 2];

    Spectrum contrib;

    if(cam_end.is_entity())
    {
        const AreaLight *light = cam_end.entity->as_light();
        if(!light)
            return {};

        const Spectrum radiance = light->radiance(cam_end.pos, cam_end.nor, cam_bend.pos - cam_end.pos);
        contrib = radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
    }
    else
    {
        Spectrum radiance;
        if(auto light = connected_path.scene.envir_light())
            radiance = light->radiance(cam_bend.pos, cam_end.pos);
        contrib = radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
    }

    if(!contrib)
        return {};

    const real weight = weight_sx_t0_path(connected_path);
    return weight * contrib;
}

Spectrum BDPTRenderer::contrib_sx_t1_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s >= 2 && connected_path.t == 1);

    const int s = connected_path.s;
    const Vertex &cam_end = connected_path.cam_subpath[s - 1];
    const Vertex &cam_bend = connected_path.cam_subpath[s - 2];

    if(!cam_end.is_entity())
        return {};

    const auto lht_sam_wi = connected_path.light->sample(cam_end.pos, connected_path.sampler.sample5());
    if(!lht_sam_wi.radiance)
        return {};

    if(!connected_path.scene.visible(cam_end.pos, lht_sam_wi.pos))
        return {};

    const Spectrum bsdf = cam_end.bsdf->eval(lht_sam_wi.ref_to_light(), cam_bend.pos - cam_end.pos, TransportMode::Radiance);

    const real proj_pdf = lht_sam_wi.pdf / std::abs(cos(cam_end.nor, lht_sam_wi.ref_to_light()));
    const Spectrum contrib = bsdf * cam_end.accu_bsdf * lht_sam_wi.radiance
                           / (connected_path.select_light_pdf * cam_end.accu_proj_pdf * proj_pdf);

    if(!contrib)
        return {};

    const real weight = weight_sx_t1_path(connected_path, lht_sam_wi);
    return weight * contrib;
}

Spectrum BDPTRenderer::contrib_sx_tx_path(const ConnectedPath &connected_path) const
{
    const int s = connected_path.s;
    const int t = connected_path.t;
    assert(s > 1 && t > 1);

    const Vertex &cam_end = connected_path.cam_subpath[s - 1];
    const Vertex &lht_end = connected_path.lht_subpath[t - 1];
    if(!cam_end.is_entity())
        return {};

    if(!connected_path.scene.visible(cam_end.pos, lht_end.pos))
        return {};

    const Vertex &cam_bend = connected_path.cam_subpath[s - 2];
    const Vertex &lht_bend = connected_path.lht_subpath[t - 2];

    const real pdf = cam_end.accu_proj_pdf * lht_end.accu_proj_pdf;

    const Vec3 cam_end_to_lht_end = lht_end.pos - cam_end.pos;
    const Spectrum cam_bsdf = cam_end.bsdf->eval(
        cam_end_to_lht_end, cam_bend.pos - cam_end.pos, TransportMode::Radiance);
    const Spectrum lht_bsdf = lht_end.bsdf->eval(
        -cam_end_to_lht_end, lht_bend.pos - lht_end.pos, TransportMode::Importance);

    const Spectrum contrib = cam_bsdf * lht_bsdf * cam_end.accu_bsdf * lht_end.accu_bsdf
                           * G(cam_end.pos, lht_end.pos, cam_end.nor, lht_end.nor) / pdf;

    if(contrib.is_black())
        return {};

    const real weight = weight_sx_tx_path(connected_path);
    return contrib * weight;
}

real BDPTRenderer::weight_common(const ConnectedPath &connected_path, real G_between_subpaths) const
{
    const int s = connected_path.s;
    const int t = connected_path.t;
    assert(s >= 1 && s + t >= 3);

    const Vertex *C = connected_path.cam_subpath;
    const Vertex *L = connected_path.lht_subpath;

    real sum_pdf = 1;

    // 将light subpath逐顶点纳入camera subpath中

    real cur_pdf = 1;

    // light end
    if(t >= 2)
    {
        const real mul = z2o(L[t - 1].pdf_fwd) * z2o(G_between_subpaths);
        const real div = z2o(L[t - 1].pdf_bwd) * z2o(L[t - 1].G_with_last);

        cur_pdf *= mul / div;

        if(!L[t - 2].is_delta)
            sum_pdf += cur_pdf;
    }

    for(int i = t - 2; i >= 1; --i)
    {
        const real mul = z2o(L[i].pdf_fwd) * z2o(L[i + 1].G_with_last);
        const real div = z2o(L[i].pdf_bwd) * z2o(L[i].G_with_last);

        cur_pdf *= mul / div;

        if(!L[i].is_delta && !L[i - 1].is_delta)
            sum_pdf += cur_pdf;
    }

    // light beg
    if(t >= 1)
    {
        const real mul_G = t == 1 ? G_between_subpaths : L[1].G_with_last;
        const real mul = z2o(L[0].pdf_fwd) * z2o(mul_G);
        const real div = z2o(L[0].pdf_bwd);

        cur_pdf *= mul / div;

        if(!L[0].is_delta)
            sum_pdf += cur_pdf;
    }

    // 将camera subpath逐顶点纳入light subpath中

    cur_pdf = 1;

    // camera end
    if(s >= 2)
    {
        const real mul = z2o(C[s - 1].pdf_bwd) * z2o(G_between_subpaths);
        const real div = z2o(C[s - 1].pdf_fwd) * z2o(C[s - 1].G_with_last);

        cur_pdf *= mul / div;

        if(!C[s - 2].is_delta)
            sum_pdf += cur_pdf;
    }

    for(int i = s - 2; i >= 1; --i)
    {
        const real mul = z2o(C[i].pdf_bwd) * z2o(C[i + 1].G_with_last);
        const real div = z2o(C[i].pdf_fwd) * z2o(C[i].G_with_last);

        cur_pdf *= mul / div;

        if(!C[i].is_delta && !C[i - 1].is_delta)
            sum_pdf += cur_pdf;
    }

    return 1 / sum_pdf;
}

real BDPTRenderer::weight_s1_tx_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s == 1 && connected_path.t >= 2);

    if(!params_.use_mis)
        return real(1) / (1 + connected_path.t);

    const int t = connected_path.t;
    Vertex *lht_subpath = connected_path.lht_subpath;
    const Vertex &cam_beg = connected_path.cam_subpath[0];
    Vertex &lht_end = lht_subpath[t - 1];

    TmpAssign a0;
    {
        const Vec3 cam_beg_to_lht_end = lht_end.pos - cam_beg.pos;
        const CameraWePDFResult cam_we_pdf = connected_path.camera->pdf_we(cam_beg.pos, cam_beg_to_lht_end);

        a0 = { &lht_end.pdf_fwd, cam_we_pdf.pdf_dir / std::abs(cos(cam_beg.nor, cam_beg_to_lht_end)) };
    }
    AGZ_UNACCESSED(a0);

    TmpAssign a1;
    {
        const Vec3 wo = cam_beg.pos - lht_end.pos;
        const Vec3 wi = lht_subpath[t - 2].pos - lht_end.pos;
        const real pdf = lht_end.bsdf->pdf(wi, wo, TransportMode::Radiance);
        const real proj_pdf = pdf / std::abs(cos(lht_end.nor, wi));

        a1 = { &lht_subpath[t - 2].pdf_fwd, proj_pdf };
    }
    AGZ_UNACCESSED(a1);

    const real connected_G = G(cam_beg.pos, lht_end.pos, cam_beg.nor, lht_end.nor);

    return weight_common(connected_path, connected_G);
}

real BDPTRenderer::weight_sx_t0_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s >= 3 && connected_path.t == 0);

    if(!params_.use_mis)
        return real(1) / connected_path.s;

    Vertex *cam_subpath = connected_path.cam_subpath;
    Vertex &cam_end = cam_subpath[connected_path.s - 1];
    Vertex &cam_bend = cam_subpath[connected_path.s - 2];

    if(cam_end.is_entity())
    {
        const Vec3 bend_to_end = cam_end.pos - cam_bend.pos;

        const AreaLight *light = cam_end.entity->as_light();
        const real select_light_pdf = connected_path.scene.light_pdf(light);
        const LightEmitPDFResult emit_pdf = light->emit_pdf(cam_end.pos, -bend_to_end, cam_end.nor);

        TmpAssign a0 = { &cam_end.pdf_bwd, select_light_pdf * emit_pdf.pdf_pos };

        TmpAssign a1 = { &cam_bend.pdf_bwd, emit_pdf.pdf_dir / std::abs(cos(cam_end.nor, bend_to_end)) };

        return weight_common(connected_path, 1);
    }

    const EnvirLight *light = connected_path.scene.envir_light();
    assert(light != nullptr);

    const real select_light_pdf = connected_path.scene.light_pdf(light);
    const LightEmitPDFResult emit_pdf = light->emit_pdf({}, -cam_end.pos, {});
    const LightEmitPosResult emit_pos = light->emit_pos(cam_bend.pos, cam_end.pos);

    TmpAssign a0 = { &cam_end.pdf_bwd, select_light_pdf * emit_pdf.pdf_pos };
    TmpAssign a1 = { &cam_bend.pdf_bwd, emit_pdf.pdf_dir };
    TmpAssign a2 = { &cam_end.G_with_last, G(cam_bend.pos, emit_pos.pos, cam_bend.nor, emit_pos.nor) };

    return weight_common(connected_path, 1);
}

real BDPTRenderer::weight_sx_t1_path(const ConnectedPath &connected_path, const LightSampleResult &lht_sam_wi) const
{
    assert(connected_path.s >= 2 && connected_path.t == 1);

    if(!params_.use_mis)
        return real(1) / (connected_path.s + 1);

    const int s = connected_path.s;
    Vertex *cam_subpath = connected_path.cam_subpath;
    Vertex &cam_end = cam_subpath[s - 1];
    Vertex &cam_bend = cam_subpath[s - 2];
    Vertex &lht_vtx = connected_path.lht_subpath[0];

    const LightEmitPDFResult emit_pdf = connected_path.light->emit_pdf(lht_sam_wi.pos, -lht_sam_wi.ref_to_light(), lht_sam_wi.nor);

    TmpAssign a0;
    {
        const real lht_vtx_pa = connected_path.select_light_pdf * emit_pdf.pdf_pos;
        a0 = { &lht_vtx.pdf_bwd, lht_vtx_pa };
    }
    AGZ_UNACCESSED(a0);

    TmpAssign a1;
    {
        const real pdf = cam_end.bsdf->pdf(lht_sam_wi.ref_to_light(), cam_bend.pos - cam_end.pos, TransportMode::Radiance);
        const real proj_pdf = pdf / std::abs(cos(cam_end.nor, lht_sam_wi.ref_to_light()));
        a1 = { &lht_vtx.pdf_fwd, proj_pdf };
    }
    AGZ_UNACCESSED(a1);

    TmpAssign a2;
    {
        const real proj_pdf = emit_pdf.pdf_dir / std::abs(cos(lht_sam_wi.nor, -lht_sam_wi.ref_to_light()));
        a2 = { &cam_end.pdf_bwd, proj_pdf };
    }
    AGZ_UNACCESSED(a2);

    TmpAssign a3;
    {
        const real pdf = cam_end.bsdf->pdf(cam_bend.pos - cam_end.pos, lht_sam_wi.ref_to_light(), TransportMode::Importance);
        const real proj_pdf = pdf / std::abs(cos(cam_end.nor, cam_bend.pos - cam_end.pos));
        a3 = { &cam_bend.pdf_bwd, proj_pdf };
    }
    AGZ_UNACCESSED(a3);

    const real connected_G = G(cam_end.pos, lht_sam_wi.pos, cam_end.nor, lht_sam_wi.nor);

    return weight_common(connected_path, connected_G);
}

real BDPTRenderer::weight_sx_tx_path(const ConnectedPath &connected_path) const
{
    assert(connected_path.s > 1 && connected_path.t > 1);

    if(!params_.use_mis)
        return real(1) / (connected_path.s + connected_path.t);

    // ..., a, b | c, d, ...

    const int s = connected_path.s;
    const int t = connected_path.t;
    Vertex *cam_subpath = connected_path.cam_subpath;
    Vertex *lht_subpath = connected_path.lht_subpath;
    Vertex &a = cam_subpath[s - 2];
    Vertex &b = cam_subpath[s - 1];
    Vertex &c = lht_subpath[t - 1];
    Vertex &d = lht_subpath[t - 2];

    TmpAssign a0;
    {
        const real pdf = b.bsdf->pdf(c.pos - b.pos, a.pos - b.pos, TransportMode::Radiance);
        const real proj_pdf = pdf / std::abs(cos(b.nor, c.pos - b.pos));
        a0 = { &c.pdf_fwd, proj_pdf };
    }
    AGZ_UNACCESSED(a0);

    TmpAssign a1;
    {
        const real pdf = c.bsdf->pdf(d.pos - c.pos, b.pos - c.pos, TransportMode::Radiance);
        const real proj_pdf = pdf / std::abs(cos(c.nor, d.pos - c.pos));
        a1 = { &d.pdf_fwd, proj_pdf };
    }
    AGZ_UNACCESSED(a1);

    TmpAssign a2;
    {
        const real pdf = c.bsdf->pdf(b.pos - c.pos, d.pos - c.pos, TransportMode::Importance);
        const real proj_pdf = pdf / std::abs(cos(c.nor, b.pos - c.pos));
        a2 = { &b.pdf_bwd, proj_pdf };
    }
    AGZ_UNACCESSED(a2);

    TmpAssign a3;
    {
        const real pdf = b.bsdf->pdf(a.pos - b.pos, c.pos - b.pos, TransportMode::Importance);
        const real proj_pdf = pdf / std::abs(cos(b.nor, a.pos - b.pos));
        a3 = { &a.pdf_bwd, proj_pdf };
    }
    AGZ_UNACCESSED(a3);

    const real connected_G = G(b.pos, c.pos, b.nor, c.nor);

    return weight_common(connected_path, connected_G);
}

std::shared_ptr<Renderer> create_bdpt_renderer(const BDPTRendererParams &params)
{
    return std::make_shared<BDPTRenderer>(params);
}

AGZ_TRACER_END
