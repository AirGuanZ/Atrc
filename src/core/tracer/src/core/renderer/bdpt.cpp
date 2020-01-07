//#include <atomic>
//#include <mutex>
//#include <thread>
//
//#include <agz/tracer/core/bsdf.h>
//#include <agz/tracer/core/camera.h>
//#include <agz/tracer/core/entity.h>
//#include <agz/tracer/core/light.h>
//#include <agz/tracer/core/material.h>
//#include <agz/tracer/core/reporter.h>
//#include <agz/tracer/core/renderer.h>
//#include <agz/tracer/core/scene.h>
//#include <agz/tracer/factory/raw/renderer.h>
//#include <agz/utility/thread.h>
//
//AGZ_TRACER_BEGIN
//
//namespace
//{
//    // 路径顶点
//    struct Vertex
//    {
//        Vec3 pos;                       // 有交点是表示位置，否则表示ray.d
//        Vec3 nor;                       // 法线，有交点时有效
//        Spectrum accu_bsdf;             // 从采样起点到该顶点的前一个顶点间的累计bsdf，有无交点都有效
//        real accu_proj_pdf   = 0;       // 从采样起点到该顶点的累计proj pdf，有无交点都有效
//        real pdf_fwd_dest    = 0;       // 沿forward方向从前一个顶点采样到该顶点的pdf，有无交点都有效
//        real pdf_bwd_dest    = 0;       // 沿backward方向从前一个顶点采样到该顶点的pdf
//        real G_with_last     = 0;       // 和路径中上一个点间的G值，有交点时有效
//        const Entity *entity = nullptr; // 若为nullptr，则表示无交点，此时position记录ray.d
//        const BSDF   *bsdf   = nullptr; // 当前顶点的bsdf，有交点时有效
//        bool is_delta        = false;   // 是否是一个delta顶点，有交点时有效
//    };
//
//    real G(const Vec3 &a, const Vec3 &b, const Vec3 &na, const Vec3 &nb) noexcept
//    {
//        const Vec3 d = a - b;
//        return std::abs(cos(d, na) * cos(d, nb) / d.length_square());
//    }
//
//    real G(const Vertex &a, const Vertex &b) noexcept
//    {
//        return G(a.pos, b.pos, a.nor, b.nor);
//    }
//
//    real z2o(real x) noexcept
//    {
//        return x == 0 ? 1 : x;
//    }
//}
//
///**
// * @brief bidirectional path tracing renderer
// */
//class BDPTRenderer : public Renderer
//{
//public:
//
//    explicit BDPTRenderer(const BDPTRendererParams &params);
//
//    //void render(Scene &scene, ProgressReporter &reporter, Film *film) override;
//    RenderTarget render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter) override;
//
//private:
//
//    struct ConnectParams
//    {
//        const Scene &scene;
//        const Light *light;
//        const Camera *camera;
//        const real select_light_pdf;
//        const LightEmitResult *lht_emit;
//        const Vertex *cam_subpath;
//        const Vertex *lht_subpath;
//        int s;
//        int t;
//        const Vec2i &full_res;
//        Sampler &sampler;
//        FilmGrid *bwd_film_grid;
//    };
//
//    // 返回backward tracing的粒子总数
//    int render_grid(
//        const Scene &scene, Sampler &sampler,
//        FilmGrid *film_grid, FilmGrid *bwd_film_grid, const Vec2i &full_res) const;
//
//    int build_camera_subpath(
//        const CameraSampleWeResult &cam_sam, const CameraWePDFResult &cam_pdf,
//        const Scene &scene, Sampler &sampler, Arena &arena,
//        GBufferPixel &gpixel, std::vector<Vertex> &cam_subpath) const;
//
//    int build_light_subpath(
//        const LightEmitResult &lht_sam, real select_lht_pdf,
//        const Scene &scene, Sampler &sampler, Arena &arena,
//        std::vector<Vertex> &lht_subpath) const;
//
//    real weight_s1_tx_path(const ConnectParams &params, const CameraSampleWiResult &cam_sam) const;
//    real weight_sx_t0_path(const ConnectParams &params) const;
//    real weight_sx_t1_path(const ConnectParams &params) const;
//    real weight_sx_tx_path(const ConnectParams &params) const;
//
//    Spectrum contrib_s2_t0_path(const ConnectParams &params) const;
//    Spectrum contrib_s1_tx_path(const ConnectParams &params) const;
//    Spectrum contrib_sx_t0_path(const ConnectParams &params) const;
//    Spectrum contrib_sx_t1_path(const ConnectParams &params) const;
//    Spectrum contrib_sx_tx_path(const ConnectParams &params) const;
//
//    Spectrum connect_subpath(const ConnectParams &params) const;
//
//    int sample_path(
//        int px, int py,
//        const Scene &scene, Sampler &sampler, Arena &arena,
//        FilmGrid *film_grid, FilmGrid *bwd_film_grid, const Vec2i &full_res,
//        std::vector<Vertex> &cam_subpath, std::vector<Vertex> &lht_subpath) const;
//
//    BDPTRendererParams params_;
//};
//
//std::shared_ptr<Renderer> create_bdpt_renderer(const BDPTRendererParams &params)
//{
//    return std::make_shared<BDPTRenderer>(params);
//}
//
//BDPTRenderer::BDPTRenderer(const BDPTRendererParams &params)
//    : params_(params)
//{
//    
//}
//
//RenderTarget BDPTRenderer::render(const FilmFilterApplier &filter, Scene &scene, ProgressReporter &reporter)
//{
//    return {};
//
//    /*scene.start_rendering();
//    reporter.begin();
//    reporter.new_stage();
//
//    // 计算任务数量
//
//    const auto [film_width, film_height] = film->resolution();
//    const int x_task_count = (film_width  + params_.task_grid_size - 1) / params_.task_grid_size;
//    const int y_task_count = (film_height + params_.task_grid_size - 1) / params_.task_grid_size;
//    const int total_task_count = x_task_count * y_task_count;
//
//    // 定义渲染任务
//
//    std::mutex reporter_mutex;
//    std::atomic<int> next_task_id = 0;
//    std::atomic<int> total_particle_count = 0;
//
//    auto render_func = [
//        film,
//        &scene,
//        &reporter,
//        &reporter_mutex,
//        &total_particle_count,
//        &next_task_id,
//        x_task_count,
//        total_task_count,
//        task_grid_size = params_.task_grid_size,
//        this
//    ] (Sampler *sampler, FilmGrid *backward_film_grid)
//    {
//            const Vec2i film_res = film->resolution();
//
//        for(;;)
//        {
//            const int task_id = next_task_id++;
//            if(task_id >= total_task_count)
//                return;
//
//            const int grid_y_index = task_id / x_task_count;
//            const int grid_x_index = task_id % x_task_count;
//
//            const int x_beg = grid_x_index * task_grid_size;
//            const int y_beg = grid_y_index * task_grid_size;
//            const int x_end = (std::min)(x_beg + task_grid_size, film_res.x);
//            const int y_end = (std::min)(y_beg + task_grid_size, film_res.y);
//
//            auto film_grid = film->new_grid(x_beg, x_end, y_beg, y_end);
//            total_particle_count += this->render_grid(
//                scene, *sampler, film_grid.get(), backward_film_grid, film_res);
//            film->merge_grid(*film_grid);
//
//            const real percent = real(100) * (task_id + 1) / total_task_count;
//            std::lock_guard lk(reporter_mutex);
//            reporter.progress(percent);
//        }
//    };
//
//    // 执行渲染任务
//
//    const int actual_worker_count = thread::actual_worker_count(params_.worker_count);
//    Arena sampler_arena;
//
//    std::vector<std::thread> threads;
//    std::vector<std::unique_ptr<FilmGrid>> backward_film_grids;
//    threads.reserve(actual_worker_count);
//    backward_film_grids.reserve(actual_worker_count);
//
//    for(int i = 0; i < actual_worker_count; ++i)
//    {
//        auto sampler = params_.sampler_prototype->clone(i, sampler_arena);
//        auto film_grid = film->new_grid(0, film_width, 0, film_height);
//        backward_film_grids.push_back(std::move(film_grid));
//        threads.emplace_back(render_func, sampler, backward_film_grids.back().get());
//    }
//
//    for(auto &t : threads)
//        t.join();
//
//    // 合并渲染结果
//
//    real backward_to_forward = 0;
//    if(total_particle_count)
//    {
//        backward_to_forward = params_.sampler_prototype->get_spp()
//            * static_cast<real>(film->resolution().product()) / total_particle_count;
//    }
//
//    for(auto &backward_film_grid : backward_film_grids)
//        film->merge_grid(*backward_film_grid, backward_to_forward);
//
//    // 设置权重
//
//    film->set_scale(1 / static_cast<real>(params_.sampler_prototype->get_spp()));
//
//    reporter.end_stage();
//    reporter.end();
//    reporter.message("total time: " + std::to_string(reporter.total_seconds()) + "s");*/
//}
//
//// 返回backward tracing的粒子总数
//int BDPTRenderer::render_grid(
//    const Scene &scene, Sampler &sampler,
//    FilmGrid *film_grid, FilmGrid *bwd_film_grid, const Vec2i &full_res) const
//{
//    int particle_count = 0;
//
//    Arena arena;
//    const int x_beg = film_grid->sample_x_beg();
//    const int x_end = film_grid->sample_x_end();
//    const int y_beg = film_grid->sample_y_beg();
//    const int y_end = film_grid->sample_y_end();
//
//    std::vector<Vertex> cam_subpath(params_.cam_max_vtx_cnt);
//    std::vector<Vertex> lht_subpath(params_.lht_max_vtx_cnt);
//
//    for(int py = y_beg; py < y_end; ++py)
//    {
//        for(int px = x_beg; px < x_end; ++px)
//        {
//            sampler.start_pixel(px, py);
//            do
//            {
//                particle_count += sample_path(
//                    px, py,
//                    scene, sampler, arena,
//                    film_grid, bwd_film_grid, full_res,
//                    cam_subpath, lht_subpath);
//
//                arena.release();
//
//            } while(sampler.next_sample());
//        }
//    }
//
//    return particle_count;
//}
//
//int BDPTRenderer::build_camera_subpath(
//    const CameraSampleWeResult &cam_sam, const CameraWePDFResult &cam_pdf,
//    const Scene &scene, Sampler &sampler, Arena &arena,
//    GBufferPixel &gpixel, std::vector<Vertex> &cam_subpath) const
//{
//    const auto cam = scene.get_camera();
//    
//    Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);
//
//    // 之前所有顶点的bsdf累积结果，包括camera we
//    Spectrum cam_accu_bsdf = cam->eval_we(cam_sam.pos_on_cam, cam_sam.pos_to_out).we;
//
//    // 上一个顶点sample到当前ray的proj pdf
//    real cam_fwd_proj_pdf =
//        cam_pdf.pdf_dir / std::abs(cos(cam_sam.nor_on_cam, cam_sam.pos_to_out));
//
//    // 之前所有顶点的proj_pdf累积结果，包括pdf_lens
//    real cam_accu_proj_pdf = cam_pdf.pdf_pos * cam_fwd_proj_pdf;
//
//    // 上一个顶点的位置
//    Vec3 last_pos = cam_sam.pos_on_cam;
//
//    // 上一个顶点的法线
//    Vec3 last_nor = cam_sam.nor_on_cam;
//
//    int cam_vtx_cnt = 1;
//    while(cam_vtx_cnt < params_.cam_max_vtx_cnt)
//    {
//        EntityIntersection inct;
//        if(!scene.closest_intersection(ray, &inct))
//        {
//            Vertex new_vertex;
//            new_vertex.pos           = ray.d;
//            new_vertex.nor           = {};
//            new_vertex.accu_bsdf     = cam_accu_bsdf;
//            new_vertex.accu_proj_pdf = cam_accu_proj_pdf;
//            new_vertex.pdf_fwd_dest  = cam_fwd_proj_pdf;
//            new_vertex.pdf_bwd_dest  = 0;
//
//            cam_subpath[cam_vtx_cnt++] = new_vertex;
//            break;
//        }
//
//        const ShadingPoint shd = inct.material->shade(inct, arena);
//
//        // 填充gbuffer
//
//        if(cam_vtx_cnt == 1)
//        {
//            gpixel.position = inct.pos;
//            gpixel.normal   = shd.shading_normal;
//            gpixel.albedo   = shd.bsdf->albedo();
//            gpixel.denoise  = inct.entity->get_no_denoise_flag() ? real(0) : real(1);
//            gpixel.depth    = ray.d.length() * inct.t;
//        }
//
//        // 追加新顶点
//
//        Vertex new_vertex;
//        new_vertex.pos           = inct.pos;
//        new_vertex.nor           = inct.geometry_coord.z;
//        new_vertex.accu_bsdf     = cam_accu_bsdf;
//        new_vertex.accu_proj_pdf = cam_accu_proj_pdf;
//        new_vertex.pdf_fwd_dest  = cam_fwd_proj_pdf;
//        new_vertex.pdf_bwd_dest  = 0;
//        new_vertex.G_with_last   = G(last_pos, inct.pos, last_nor, inct.geometry_coord.z);
//        new_vertex.entity        = inct.entity;
//        new_vertex.bsdf          = shd.bsdf;
//        new_vertex.is_delta      = shd.bsdf->is_delta();
//
//        cam_subpath[cam_vtx_cnt++] = new_vertex;
//
//        last_pos = inct.pos;
//        last_nor = inct.geometry_coord.z;
//
//        // 采样bsdf
//
//        const BSDFSampleResult bsdf_sample = shd.bsdf->sample(inct.wr, TransportMode::Radiance, sampler.sample3());
//        if(!bsdf_sample.f)
//            break;
//
//        const real proj_pdf = bsdf_sample.pdf / std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));
//        cam_accu_bsdf *= bsdf_sample.f;
//        cam_accu_proj_pdf *= proj_pdf;
//        cam_fwd_proj_pdf = proj_pdf;
//
//        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
//    }
//
//    // 填充backward proj pdf
//
//    // 除去表示无交点的末端顶点后，末尾的前两个顶点都无法计算backward pdf
//    const int index_end = cam_subpath[cam_vtx_cnt - 1].entity ?
//                          cam_vtx_cnt - 2 : cam_vtx_cnt - 3;
//
//    for(int i = 1; i < index_end; ++i)
//    {
//        Vertex &cur_vtx  = cam_subpath[i];
//        const Vertex &fwd_vtx  = cam_subpath[i + 1];
//        const Vertex &fwd2_vtx = cam_subpath[i + 2];
//        const real pdf = fwd_vtx.bsdf->pdf(fwd2_vtx.pos - fwd_vtx.pos, cur_vtx.pos - fwd_vtx.pos, TransportMode::Importance);
//        const real cos = std::abs(math::cos(fwd_vtx.nor, cur_vtx.pos - fwd_vtx.pos));
//        cur_vtx.pdf_bwd_dest = pdf / cos;
//    }
//
//    return cam_vtx_cnt;
//}
//
//int BDPTRenderer::build_light_subpath(
//    const LightEmitResult &lht_emit, real select_lht_pdf,
//    const Scene &scene, Sampler &sampler, Arena &arena,
//    std::vector<Vertex> &lht_subpath) const
//{
//    Ray ray(lht_emit.position + EPS * lht_emit.direction, lht_emit.direction);
//
//    // 之前所有顶点的bsdf累积结果，包括light radiance
//    Spectrum lht_accu_bsdf = lht_emit.radiance;
//
//    // 上一个顶点sample到当前ray的proj pdf
//    real lht_bwd_proj_pdf =
//        lht_emit.pdf_dir / std::abs(cos(lht_emit.direction, lht_emit.normal));
//    
//    // 之前所有顶点的proj pdf累积结果，包括light pdf_pos
//    real lht_accu_proj_pdf = select_lht_pdf * lht_emit.pdf_pos * lht_bwd_proj_pdf;
//
//    // 上一个顶点的位置
//    Vec3 last_pos = lht_emit.position;
//
//    // 上一个顶点的法线
//    Vec3 last_nor = lht_emit.normal;;
//
//    int lht_vtx_cnt = 1;
//    while(lht_vtx_cnt < params_.lht_max_vtx_cnt)
//    {
//        // light ray不会击中camera，故此处不需要记录light_ray落空的末端顶点
//        EntityIntersection inct;
//        if(!scene.closest_intersection(ray, &inct))
//            break;
//
//        const ShadingPoint shd = inct.material->shade(inct, arena);
//
//        // 追加新顶点
//
//        Vertex new_vertex;
//        new_vertex.pos           = inct.pos;
//        new_vertex.nor           = inct.geometry_coord.z;
//        new_vertex.accu_bsdf     = lht_accu_bsdf;
//        new_vertex.accu_proj_pdf = lht_accu_proj_pdf;
//        new_vertex.pdf_fwd_dest  = 0;
//        new_vertex.pdf_bwd_dest  = lht_bwd_proj_pdf;
//        new_vertex.G_with_last   = G(last_pos, inct.pos, last_nor, inct.geometry_coord.z);
//        new_vertex.entity        = inct.entity;
//        new_vertex.bsdf          = shd.bsdf;
//        new_vertex.is_delta      = shd.bsdf->is_delta();
//
//        lht_subpath[lht_vtx_cnt++] = new_vertex;
//
//        last_pos = inct.pos;
//        last_nor = inct.geometry_coord.z;
//
//        // 采样bsdf
//
//        const BSDFSampleResult bsdf_sample = shd.bsdf->sample(inct.wr, TransportMode::Importance, sampler.sample3());
//        if(!bsdf_sample.f)
//            break;
//
//        const real proj_pdf = bsdf_sample.pdf / std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));
//        lht_accu_bsdf *= bsdf_sample.f;
//        lht_accu_proj_pdf *= proj_pdf;
//        lht_bwd_proj_pdf = proj_pdf;
//
//        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
//    }
//
//    // 填充forward proj pdf
//
//    // 末尾的前两个顶点都无法计算backward pdf
//    const int index_end = lht_vtx_cnt - 2;
//
//    for(int i = 0; i < index_end; ++i)
//    {
//        Vertex &cur_vtx = lht_subpath[i];
//        const Vertex &fwd_vtx  = lht_subpath[i + 1];
//        const Vertex &fwd2_vtx = lht_subpath[i + 2];
//        const real pdf = fwd_vtx.bsdf->pdf(fwd2_vtx.pos - fwd_vtx.pos, cur_vtx.pos - fwd_vtx.pos, TransportMode::Radiance);
//        const real cos = std::abs(math::cos(fwd_vtx.nor, cur_vtx.pos - fwd_vtx.pos));
//        cur_vtx.pdf_fwd_dest = pdf / cos;
//    }
//
//    return lht_vtx_cnt;
//}
//
//real BDPTRenderer::weight_s1_tx_path(const ConnectParams &params, const CameraSampleWiResult &cam_sam) const
//{
//    return 1 / real(1 + params.t);
//    /*if(!params_.use_mis)
//        return 1 / real(1 + params.t);
//
//    const int t = params.t;
//    const Vertex *lht_subpath = params.lht_subpath;
//    assert(params.s == 1 && t >= 2);
//
//    real sum_pdf2 = 1;
//    real cur_pdf = 1;
//
//    // 将light_end纳入camera subpath
//
//    {
//        const Vertex &lht_end = lht_subpath[t - 1];
//
//        const auto cam_pdf = params.camera->pdf_we(cam_sam.pos_on_cam, -cam_sam.ref_to_pos);
//        const real mul = cam_pdf.pdf_dir * cos(cam_sam.nor_at_pos, -cam_sam.ref_to_pos)
//                       * G(cam_sam.pos_on_cam, lht_end.pos, cam_sam.nor_at_pos, lht_end.nor);
//        const real div = lht_end.pdf_bwd_dest * lht_end.G_with_last;
//        assert(mul > 0 && div > 0);
//
//        cur_pdf *= z2o(mul) / z2o(div);
//
//        if(!lht_subpath[t - 2].is_delta)
//            sum_pdf2 += cur_pdf * cur_pdf;
//    }
//
//    // light subpath的倒数第二个顶点
//
//    if(t > 2)
//    {
//        // complete path: a, b, c, d, ...
//        // 将c从light subpath转移到camera subpath
//
//        const Vertex &b = lht_subpath[t - 1];
//        const Vertex &c = lht_subpath[t - 2];
//        const Vertex &d = lht_subpath[t - 3];
//
//        const Vec3 ba = cam_sam.ref_to_pos;
//        const Vec3 bc = c.pos - b.pos;
//        const real mul = b.bsdf->pdf(bc, ba, TransportMode::Radiance)
//                       * cos(b.nor, bc) * b.G_with_last;
//        const real div = c.pdf_bwd_dest * c.G_with_last;
//
//        cur_pdf *= z2o(mul) / z2o(div);
//
//        if(!c.is_delta && !d.is_delta)
//            sum_pdf2 += cur_pdf * cur_pdf;
//    }
//
//    // light subpath的倒数第三个顶点到正数第二个顶点
//
//    for(int lht_idx = t - 3; lht_idx >= 1; --lht_idx)
//    {
//        // complete path: ..., a, b, c, d, ...
//        // 将c从light subpath转移到camera subpath
//
//        const Vertex &b = lht_subpath[lht_idx + 1];
//        const Vertex &c = lht_subpath[lht_idx];
//        const Vertex &d = lht_subpath[lht_idx - 1];
//
//        const real mul = c.pdf_fwd_dest * b.G_with_last;
//        const real div = c.pdf_bwd_dest * c.G_with_last;
//
//        cur_pdf *= z2o(mul) / z2o(div);
//
//        if(!c.is_delta && !d.is_delta)
//            sum_pdf2 += cur_pdf * cur_pdf;
//    }
//
//    // light subpath的第一个顶点
//
//    {
//        // complete path: ..., a, b, c, d
//        // 将d从light subpath转移到camera subpath
//
//        const Vertex &c = lht_subpath[1];
//        const Vec3 cb = t > 2 ? lht_subpath[2].pos - c.pos : cam_sam.ref_to_pos;
//        const Vec3 cd = -params.lht_emit->direction;
//
//        const real mul = c.bsdf->pdf(cd, cb, TransportMode::Radiance)
//                       * cos(c.nor, cd) * c.G_with_last;
//        const real div = params.select_light_pdf * params.lht_emit->pdf_pos;
//
//        cur_pdf *= z2o(mul) / z2o(div);
//        if(!c.is_delta)
//            sum_pdf2 += cur_pdf * cur_pdf;
//    }
//
//    return 1 / sum_pdf2;*/
//}
//
//real BDPTRenderer::weight_sx_t0_path(const ConnectParams &params) const
//{
//    if(!params_.use_mis)
//        return 1 / real(params.s);
//    return 1 / real(params.s);
//
//    /*const int s = params.t;
//    const Vertex *cam_subpath = params.cam_subpath;
//    assert(s >= 3 && params.t == 0);
//
//    real cur_pdf = 1;
//    real sum_pdf2 = 1;
//
//    // 将末端顶点纳入light subpath范畴
//    // 使用light sampling
//
//    if(cam_subpath[s - 1].entity)
//    {
//        // complete path: ..., a, b, c
//        // c位于光源上，将bc转为使用light sampling得到
//
//
//    }
//    else
//    {
//        
//    }
//
//    return 1 / real(params.s);*/
//}
//
//real BDPTRenderer::weight_sx_t1_path(const ConnectParams &params) const
//{
//    if(!params_.use_mis)
//        return 1 / real(params.s + 1);
//    return 1 / real(params.s + 1);
//}
//
//real BDPTRenderer::weight_sx_tx_path(const ConnectParams &params) const
//{
//    if(!params_.use_mis)
//        return 1 / real(params.s + params.t);
//    return 1 / real(params.s + params.t);
//}
//
//Spectrum BDPTRenderer::contrib_s2_t0_path(const ConnectParams &params) const
//{
//    const auto &end_vtx = params.cam_subpath[1];
//
//    // 实体光源
//
//    if(end_vtx.entity)
//    {
//        if(auto light = end_vtx.entity->as_light())
//        {
//            Spectrum value = light->radiance(
//                end_vtx.pos, end_vtx.nor,
//                params.cam_subpath[0].pos - params.cam_subpath[1].pos);
//            value *= end_vtx.accu_bsdf / end_vtx.accu_proj_pdf;
//            return value;
//        }
//        return {};
//    }
//
//    // 环境光
//
//    Spectrum env_rad;
//    for(auto light : params.scene.nonarea_lights())
//        env_rad += light->radiance(params.cam_subpath[0].pos, end_vtx.pos);
//
//    return env_rad * end_vtx.accu_bsdf / end_vtx.accu_proj_pdf;
//}
//
//Spectrum BDPTRenderer::contrib_s1_tx_path(const ConnectParams &params) const
//{
//    assert(params.s == 1 && params.t > 1);
//    const auto &lht_end = params.lht_subpath[params.t - 1];
//
//    const CameraSampleWiResult cam_sam = params.camera->sample_wi(lht_end.pos, params.sampler.sample2());
//    if(cam_sam.we.is_black())
//        return {};
//
//    if(!params.scene.visible(cam_sam.pos_on_cam, lht_end.pos))
//        return {};
//
//    const Spectrum bsdf = lht_end.bsdf->eval(
//        cam_sam.ref_to_pos,
//        params.lht_subpath[params.t - 2].pos - lht_end.pos,
//        TransportMode::Importance);
//    if(bsdf.is_black())
//        return {};
//
//    const real cos_lht_end = std::abs(cos(lht_end.nor, cam_sam.ref_to_pos));
//    const Spectrum value = cos_lht_end * cam_sam.we * bsdf * lht_end.accu_bsdf
//                           / (lht_end.accu_proj_pdf * cam_sam.pdf);
//    const Vec2 pixel_coord = {
//        cam_sam.film_coord.x * params.full_res.x,
//        cam_sam.film_coord.y * params.full_res.y
//    };
//
//    params.bwd_film_grid->add_sample(pixel_coord, weight_s1_tx_path(params, cam_sam) * value, {});
//    return {};
//}
//
//Spectrum BDPTRenderer::contrib_sx_t0_path(const ConnectParams &params) const
//{
//    assert(params.s > 2);
//    const auto &cam_end = params.cam_subpath[params.s - 1];
//    Spectrum value;
//
//    if(cam_end.entity)
//    {
//        if(auto area_light = cam_end.entity->as_light())
//        {
//            const Spectrum radiance = area_light->radiance(
//                cam_end.pos, cam_end.nor, params.cam_subpath[params.s - 2].pos - cam_end.pos);
//            value = radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
//        }
//    }
//    else
//    {
//        const auto &real_end = params.cam_subpath[params.s - 2];
//        for(auto nonarea_light : params.scene.nonarea_lights())
//            value += nonarea_light->radiance(real_end.pos, cam_end.pos);
//        value *= cam_end.accu_bsdf / cam_end.accu_proj_pdf;
//    }
//
//    return weight_sx_t0_path(params) * value;
//}
//
//Spectrum BDPTRenderer::contrib_sx_t1_path(const ConnectParams &params) const
//{
//    assert(params.s > 1);
//    const auto &cam_end = params.cam_subpath[params.s - 1];
//    if(!cam_end.entity)
//        return {};
//
//    const auto light_sample = params.light->sample(cam_end.pos, params.sampler.sample5());
//    if(light_sample.radiance.is_black())
//        return {};
//
//    if(!params.scene.visible(cam_end.pos, light_sample.pos))
//        return {};
//
//    const Spectrum bsdf = cam_end.bsdf->eval(
//        light_sample.ref_to_light(),
//        params.cam_subpath[params.s - 2].pos - cam_end.pos,
//        TransportMode::Radiance);
//    if(bsdf.is_black())
//        return {};
//
//    const real cos_cam_end = std::abs(cos(cam_end.nor, light_sample.ref_to_light()));
//    const Spectrum value = cos_cam_end * light_sample.radiance * bsdf * cam_end.accu_bsdf
//                         / (cam_end.accu_proj_pdf * light_sample.pdf * params.select_light_pdf);
//
//    return weight_sx_t1_path(params) * value;
//}
//
//Spectrum BDPTRenderer::contrib_sx_tx_path(const ConnectParams &params) const
//{
//    assert(params.s > 1 && params.t > 1);
//    const auto &cam_end = params.cam_subpath[params.s - 1];
//    const auto &lht_end = params.lht_subpath[params.t - 1];
//    if(!cam_end.entity)
//        return {};
//
//    if(!params.scene.visible(cam_end.pos, lht_end.pos))
//        return {};
//
//    const Vec3 cam_end_to_lht_end = lht_end.pos - cam_end.pos;
//    const Spectrum bsdf_cam_end = cam_end.bsdf->eval(
//        cam_end_to_lht_end,
//        params.cam_subpath[params.s - 2].pos - cam_end.pos,
//        TransportMode::Radiance);
//    const Spectrum bsdf_lht_end = lht_end.bsdf->eval(
//        -cam_end_to_lht_end,
//        params.lht_subpath[params.t - 2].pos - lht_end.pos,
//        TransportMode::Importance);
//
//    const real g = G(cam_end, lht_end);
//
//    const Spectrum value = g * bsdf_cam_end * bsdf_lht_end * cam_end.accu_bsdf * lht_end.accu_bsdf
//                         / (cam_end.accu_proj_pdf * lht_end.accu_proj_pdf);
//
//    return weight_sx_tx_path(params) * value;
//}
//
//Spectrum BDPTRenderer::connect_subpath(const ConnectParams &params) const
//{
//    assert(params.s > 0 && params.t >= 0);
//    
//    // 2顶点的路径只使用native path tracing这一种采样方法
//
//    if(params.s == 2 && params.t == 0)
//        return contrib_s2_t0_path(params);
//
//    // 忽略其他2顶点的采样方法，以及1顶点的情形
//
//    if(params.s + params.t <= 2)
//        return {};
//
//    // 对s == 1的情形，使用专门的camera sampling来构造路径
//
//    if(params.s == 1)
//        return contrib_s1_tx_path(params);
//
//    // 处理t == 0的情形
//
//    if(params.t == 0)
//        return contrib_sx_t0_path(params);
//
//    // 对t == 1的情形，使用专门的light sampling来构造路径
//
//    if(params.t == 1)
//        return contrib_sx_t1_path(params);
//
//    // 处理一般情形
//
//    return contrib_sx_tx_path(params);
//}
//
//int BDPTRenderer::sample_path(
//    int px, int py,
//    const Scene &scene, Sampler &sampler, Arena &arena,
//    FilmGrid *film_grid, FilmGrid *bwd_film_grid, const Vec2i &full_res,
//    std::vector<Vertex> &cam_subpath, std::vector<Vertex> &lht_subpath) const
//{
//    const Camera *camera = scene.get_camera();
//    GBufferPixel gpixel;
//
//    // sample camera we
//
//    const Sample2 film_sam = sampler.sample2();
//
//    const Vec2 pixel_coord = {
//        px + film_sam.u,
//        py + film_sam.v
//    };
//
//    const Vec2 film_coord = {
//        pixel_coord.x / full_res.x,
//        pixel_coord.y / full_res.y
//    };
//
//    const auto cam_sam = camera->sample_we(film_coord, sampler.sample2());
//    const auto cam_pdf = camera->pdf_we(cam_sam.pos_on_cam, cam_sam.pos_to_out);
//
//    cam_subpath[0].pos           = cam_sam.pos_on_cam;
//    cam_subpath[0].nor           = cam_sam.nor_on_cam;
//    cam_subpath[0].accu_bsdf     = Spectrum(0);
//    cam_subpath[0].accu_proj_pdf = 0;
//    cam_subpath[0].pdf_fwd_dest  = 0;
//    cam_subpath[0].pdf_bwd_dest  = 0;
//    
//    // generate camera subpath
//
//    const int cam_vtx_cnt = build_camera_subpath(
//        cam_sam, cam_pdf, scene, sampler, arena, gpixel, cam_subpath);
//
//    // sample light
//
//    const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
//    if(!light)
//        return 0;
//
//    auto lht_emit = light->emit(sampler.sample5());
//    if(lht_emit.radiance.is_black())
//        return 0;
//    lht_emit.direction = lht_emit.direction.normalize();
//
//    lht_subpath[0].pos           = lht_emit.position;
//    lht_subpath[0].nor           = lht_emit.normal;
//    lht_subpath[0].accu_bsdf     = Spectrum(0);
//    lht_subpath[0].accu_proj_pdf = 0;
//    lht_subpath[0].pdf_fwd_dest  = 0;
//    lht_subpath[0].pdf_bwd_dest  = 0;
//
//    // generate light subpath
//
//    const int lht_vtx_cnt = build_light_subpath(
//        lht_emit, select_light_pdf, scene, sampler, arena, lht_subpath);
//
//    // connect subpaths
//
//    Spectrum f;
//    for(int s = 1; s <= cam_vtx_cnt; ++s)
//    {
//        for(int t = 0; t <= lht_vtx_cnt; ++t)
//        {
//            const ConnectParams params = {
//                scene, light, camera, select_light_pdf, &lht_emit,
//                cam_subpath.data(), lht_subpath.data(),
//                s, t, full_res, sampler, bwd_film_grid
//            };
//            f += connect_subpath(params);
//        }
//    }
//
//    film_grid->add_sample(pixel_coord, f, gpixel);
//    return 1;
//}
//
//AGZ_TRACER_END
