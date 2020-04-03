#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/sampler.h>
#include <agz/tracer/core/scene.h>
#include <agz/tracer/render/bidir_path_tracing.h>

AGZ_TRACER_RENDER_BEGIN

namespace bdpt
{

namespace
{

    using TmpAssign = misc::scope_assignment_t<real>;

    real G(const Vec3 &a, const Vec3 &b,
           const Vec3 &na, const Vec3 &nb) noexcept
    {
        const Vec3 d = a - b;
        return std::abs(cos(d, na) * cos(d, nb)) / d.length_square();
    }

    real z2o(real x) noexcept
    {
        return !std::isnan(x) && x > EPS ? x : real(1);
    }

    Spectrum contrib_s2_t0_path(const ConnectedPath &connected_path);

    Spectrum contrib_s1_tx_path(
        const ConnectedPath &connected_path, Vec2 *pixel_coord);

    Spectrum contrib_sx_t0_path(const ConnectedPath &connected_path);

    Spectrum contrib_sx_t1_path(const ConnectedPath &connected_path);

    Spectrum contrib_sx_tx_path(const ConnectedPath &connected_path);

    /**
     * connected_path must contains at least 3 vertices
     *
     * pdf_bwd, pdf_fwd and is_delta in all vertices must be filled
     */
    real weight_common(
        const ConnectedPath &connected_path, real G_between_subpaths);

    real weight_s1_tx_path(const ConnectedPath &connected_path);

    real weight_sx_t0_path(const ConnectedPath &connected_path);

    real weight_sx_t1_path(
        const ConnectedPath &connected_path,
        const LightSampleResult &lht_sam_wi);

    real weight_sx_tx_path(const ConnectedPath &connected_path);

    Spectrum contrib_s2_t0_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s == 2 && connected_path.t == 0);
    
        const BDPTVertex &cam_beg = connected_path.cam_subpath[0];
        const BDPTVertex &cam_end = connected_path.cam_subpath[1];
    
        if(cam_end.is_entity())
        {
            const AreaLight *light = cam_end.entity->as_light();
            if(!light)
                return {};
    
            const Spectrum radiance = light->radiance(
                cam_end.pos, cam_end.nor,
                cam_end.uv, cam_beg.pos - cam_end.pos);
            return radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
        }
    
        Spectrum radiance;
        if(auto light = connected_path.scene.envir_light())
            radiance = light->radiance(cam_beg.pos, cam_end.pos);
    
        return radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
    }
    
    Spectrum contrib_s1_tx_path(
        const ConnectedPath &connected_path, Vec2 *pixel_coord)
    {
        assert(connected_path.s == 1 && connected_path.t >= 2);
    
        const int t = connected_path.t;
        BDPTVertex &cam_beg = connected_path.cam_subpath[0];
        const BDPTVertex &lht_end = connected_path.lht_subpath[t - 1];
        const BDPTVertex &lht_bend = connected_path.lht_subpath[t - 2];
    
        const CameraEvalWeResult cam_we = connected_path.camera->eval_we(
            cam_beg.pos, lht_end.pos - cam_beg.pos);
        if(!cam_we.we)
            return {};
    
        const Vec2 pixel = {
            cam_we.film_coord.x * connected_path.full_res.x,
            cam_we.film_coord.y * connected_path.full_res.y
        };
    
        if(!connected_path.sample_pixel_bound.contains(pixel))
            return {};
    
        if(!connected_path.scene.visible(cam_beg.pos, lht_end.pos))
            return {};
    
        const Spectrum bsdf = lht_end.bsdf->eval_all(
            lht_bend.pos - lht_end.pos, cam_beg.pos - lht_end.pos,
            TransMode::Radiance);
        if(!bsdf)
            return {};
    
        const real G_val = G(cam_beg.pos, lht_end.pos, cam_beg.nor, lht_end.nor);
        if(G_val < EPS)
            return {};
    
        const Spectrum contrib = cam_we.we * G_val * bsdf * lht_end.accu_bsdf
                               / (cam_beg.accu_proj_pdf * lht_end.accu_proj_pdf);
    
        if(!contrib.is_black())
        {
            const real weight = weight_s1_tx_path(connected_path);
            if(math::is_finite(weight) && contrib.is_finite())
            {
                *pixel_coord = { pixel.x, pixel.y };
                return weight * contrib;
            }
        }
    
        return {};
    }
    
    Spectrum contrib_sx_t0_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s >= 3);
        assert(connected_path.t == 0);
    
        const int s = connected_path.s;
        const BDPTVertex &cam_end = connected_path.cam_subpath[s - 1];
        const BDPTVertex &cam_bend = connected_path.cam_subpath[s - 2];
    
        Spectrum contrib;
    
        if(cam_end.is_entity())
        {
            const AreaLight *light = cam_end.entity->as_light();
            if(!light)
                return {};
    
            const Spectrum radiance = light->radiance(
                cam_end.pos, cam_end.nor, cam_end.uv, cam_bend.pos - cam_end.pos);
            contrib = radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
        }
        else
        {
            Spectrum radiance;
            if(auto light = connected_path.scene.envir_light())
                radiance = light->radiance(cam_bend.pos, cam_end.pos);
            contrib = radiance * cam_end.accu_bsdf / cam_end.accu_proj_pdf;
        }
    
        if(!contrib || !contrib.is_finite())
            return {};
    
        const real weight = weight_sx_t0_path(connected_path);
        return weight * contrib;
    }
    
    Spectrum contrib_sx_t1_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s >= 2 && connected_path.t == 1);
    
        const int s = connected_path.s;
        const BDPTVertex &cam_end = connected_path.cam_subpath[s - 1];
        const BDPTVertex &cam_bend = connected_path.cam_subpath[s - 2];
    
        if(!cam_end.is_entity())
            return {};
    
        const auto lht_sam_wi = connected_path.light->sample(
            cam_end.pos, connected_path.sampler.sample5());
        if(!lht_sam_wi.radiance)
            return {};
    
        if(!connected_path.scene.visible(cam_end.pos, lht_sam_wi.pos))
            return {};
    
        const Spectrum bsdf = cam_end.bsdf->eval_all(
            lht_sam_wi.ref_to_light(), cam_bend.pos - cam_end.pos, TransMode::Radiance);
    
        const real proj_pdf = lht_sam_wi.pdf
                            / std::abs(cos(cam_end.nor, lht_sam_wi.ref_to_light()));
        const Spectrum contrib = bsdf * cam_end.accu_bsdf * lht_sam_wi.radiance
                               / (connected_path.select_light_pdf
                                * cam_end.accu_proj_pdf * proj_pdf);

        if(!contrib)
            return {};
    
        const real weight = weight_sx_t1_path(connected_path, lht_sam_wi);
        return weight * contrib;
    }
    
    Spectrum contrib_sx_tx_path(const ConnectedPath &connected_path)
    {
        const int s = connected_path.s;
        const int t = connected_path.t;
        assert(s > 1 && t > 1);
    
        const BDPTVertex &cam_end = connected_path.cam_subpath[s - 1];
        const BDPTVertex &lht_end = connected_path.lht_subpath[t - 1];
        if(!cam_end.is_entity())
            return {};
    
        if(!connected_path.scene.visible(cam_end.pos, lht_end.pos))
            return {};
    
        const BDPTVertex &cam_bend = connected_path.cam_subpath[s - 2];
        const BDPTVertex &lht_bend = connected_path.lht_subpath[t - 2];
    
        const real pdf = cam_end.accu_proj_pdf * lht_end.accu_proj_pdf;
    
        const Vec3 cam_end_to_lht_end = lht_end.pos - cam_end.pos;
        const Spectrum cam_bsdf = cam_end.bsdf->eval_all(
            cam_end_to_lht_end, cam_bend.pos - cam_end.pos, TransMode::Radiance);
        const Spectrum lht_bsdf = lht_end.bsdf->eval_all(
            -cam_end_to_lht_end, lht_bend.pos - lht_end.pos, TransMode::Importance);
    
        const real g = G(cam_end.pos, lht_end.pos, cam_end.nor, lht_end.nor);
        const Spectrum contrib = cam_bsdf * lht_bsdf
                               * cam_end.accu_bsdf * lht_end.accu_bsdf
                               * g / pdf;
    
        if(contrib.is_black())
            return {};
    
        const real weight = weight_sx_tx_path(connected_path);
        return contrib * weight;
    }

    real weight_common(
        const ConnectedPath &connected_path, real G_between_subpaths)
    {
        const int s = connected_path.s;
        const int t = connected_path.t;
        assert(s >= 1 && s + t >= 3);
    
        const BDPTVertex *C = connected_path.cam_subpath;
        const BDPTVertex *L = connected_path.lht_subpath;
    
        real sum_pdf = 1;
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
    
    real weight_s1_tx_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s == 1 && connected_path.t >= 2);
    
        const int t = connected_path.t;
        BDPTVertex *lht_subpath = connected_path.lht_subpath;
        const BDPTVertex &cam_beg = connected_path.cam_subpath[0];
        BDPTVertex &lht_end = lht_subpath[t - 1];
    
        TmpAssign a0;
        {
            const Vec3 cam_beg_to_lht_end = lht_end.pos - cam_beg.pos;
            const CameraWePDFResult cam_we_pdf = connected_path.camera->pdf_we(
                cam_beg.pos, cam_beg_to_lht_end);
    
            a0 = {
                &lht_end.pdf_fwd,
                cam_we_pdf.pdf_dir / std::abs(cos(
                        cam_beg.nor, cam_beg_to_lht_end)) };
        }
        AGZ_UNACCESSED(a0);
    
        TmpAssign a1;
        {
            const Vec3 wo = cam_beg.pos - lht_end.pos;
            const Vec3 wi = lht_subpath[t - 2].pos - lht_end.pos;
            const real pdf = lht_end.bsdf->pdf_all(wi, wo);
            const real proj_pdf = pdf / std::abs(cos(lht_end.nor, wi));
    
            a1 = { &lht_subpath[t - 2].pdf_fwd, proj_pdf };
        }
        AGZ_UNACCESSED(a1);
    
        const real connected_G = G(
            cam_beg.pos, lht_end.pos, cam_beg.nor, lht_end.nor);
    
        return weight_common(connected_path, connected_G);
    }
    
    real weight_sx_t0_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s >= 3 && connected_path.t == 0);
    
        BDPTVertex *cam_subpath = connected_path.cam_subpath;
        BDPTVertex &cam_end = cam_subpath[connected_path.s - 1];
        BDPTVertex &cam_bend = cam_subpath[connected_path.s - 2];
    
        if(cam_end.is_entity())
        {
            const Vec3 bend_to_end = cam_end.pos - cam_bend.pos;
    
            const AreaLight *light = cam_end.entity->as_light();
            const real select_light_pdf = connected_path.scene.light_pdf(light);
            const LightEmitPDFResult emit_pdf = light->emit_pdf(
                cam_end.pos, -bend_to_end, cam_end.nor);
    
            TmpAssign a0 = { &cam_end.pdf_bwd, select_light_pdf * emit_pdf.pdf_pos };
    
            TmpAssign a1 = {
                &cam_bend.pdf_bwd,
                emit_pdf.pdf_dir / std::abs(cos(cam_end.nor, bend_to_end))
            };
    
            return weight_common(connected_path, 1);
        }
    
        const EnvirLight *light = connected_path.scene.envir_light();
        assert(light != nullptr);
    
        const real select_light_pdf = connected_path.scene.light_pdf(light);
        const LightEmitPDFResult emit_pdf = light->emit_pdf(
                                                {}, -cam_end.pos, {});
        const LightEmitPosResult emit_pos = light->emit_pos(
                                                cam_bend.pos, cam_end.pos);
    
        TmpAssign a0 = { &cam_end.pdf_bwd, select_light_pdf * emit_pdf.pdf_pos };
        TmpAssign a1 = { &cam_bend.pdf_bwd, emit_pdf.pdf_dir };
        TmpAssign a2 = {
            &cam_end.G_with_last,
            G(cam_bend.pos, emit_pos.pos, cam_bend.nor, emit_pos.nor)
        };
    
        return weight_common(connected_path, 1);
    }
    
    real weight_sx_t1_path(
        const ConnectedPath &connected_path,
        const LightSampleResult &lht_sam_wi)
    {
        assert(connected_path.s >= 2 && connected_path.t == 1);
    
        const int s = connected_path.s;
        BDPTVertex *cam_subpath = connected_path.cam_subpath;
        BDPTVertex &cam_end = cam_subpath[s - 1];
        BDPTVertex &cam_bend = cam_subpath[s - 2];
        BDPTVertex &lht_vtx = connected_path.lht_subpath[0];
    
        const LightEmitPDFResult emit_pdf = connected_path.light->emit_pdf(
            lht_sam_wi.pos, -lht_sam_wi.ref_to_light(), lht_sam_wi.nor);
    
        TmpAssign a0;
        {
            const real lht_vtx_pa = connected_path.select_light_pdf
                                  * emit_pdf.pdf_pos;
            a0 = { &lht_vtx.pdf_bwd, lht_vtx_pa };
        }
        AGZ_UNACCESSED(a0);
    
        TmpAssign a1;
        {
            const real pdf = cam_end.bsdf->pdf_all(
                lht_sam_wi.ref_to_light(), cam_bend.pos - cam_end.pos);
            const real proj_pdf = pdf / std::abs(cos(
                    cam_end.nor, lht_sam_wi.ref_to_light()));
            a1 = { &lht_vtx.pdf_fwd, proj_pdf };
        }
        AGZ_UNACCESSED(a1);
    
        TmpAssign a2;
        {
            const real proj_pdf = emit_pdf.pdf_dir
                                / std::abs(cos(
                                    lht_sam_wi.nor, -lht_sam_wi.ref_to_light()));
            a2 = { &cam_end.pdf_bwd, proj_pdf };
        }
        AGZ_UNACCESSED(a2);
    
        TmpAssign a3;
        {
            const real pdf = cam_end.bsdf->pdf_all(
                cam_bend.pos - cam_end.pos, lht_sam_wi.ref_to_light());
            const real proj_pdf = pdf / std::abs(cos(
                                    cam_end.nor, cam_bend.pos - cam_end.pos));
            a3 = { &cam_bend.pdf_bwd, proj_pdf };
        }
        AGZ_UNACCESSED(a3);
    
        const real connected_G = G(
            cam_end.pos, lht_sam_wi.pos, cam_end.nor, lht_sam_wi.nor);
    
        return weight_common(connected_path, connected_G);
    }
    
    real weight_sx_tx_path(const ConnectedPath &connected_path)
    {
        assert(connected_path.s > 1 && connected_path.t > 1);
    
        // ..., a, b | c, d, ...
    
        const int s = connected_path.s;
        const int t = connected_path.t;
        BDPTVertex *cam_subpath = connected_path.cam_subpath;
        BDPTVertex *lht_subpath = connected_path.lht_subpath;
        BDPTVertex &a = cam_subpath[s - 2];
        BDPTVertex &b = cam_subpath[s - 1];
        BDPTVertex &c = lht_subpath[t - 1];
        BDPTVertex &d = lht_subpath[t - 2];
    
        TmpAssign a0;
        {
            const real pdf = b.bsdf->pdf_all(c.pos - b.pos, a.pos - b.pos);
            const real proj_pdf = pdf / std::abs(cos(b.nor, c.pos - b.pos));
            a0 = { &c.pdf_fwd, proj_pdf };
        }
        AGZ_UNACCESSED(a0);
    
        TmpAssign a1;
        {
            const real pdf = c.bsdf->pdf_all(d.pos - c.pos, b.pos - c.pos);
            const real proj_pdf = pdf / std::abs(cos(c.nor, d.pos - c.pos));
            a1 = { &d.pdf_fwd, proj_pdf };
        }
        AGZ_UNACCESSED(a1);
    
        TmpAssign a2;
        {
            const real pdf = c.bsdf->pdf_all(b.pos - c.pos, d.pos - c.pos);
            const real proj_pdf = pdf / std::abs(cos(c.nor, b.pos - c.pos));
            a2 = { &b.pdf_bwd, proj_pdf };
        }
        AGZ_UNACCESSED(a2);
    
        TmpAssign a3;
        {
            const real pdf = b.bsdf->pdf_all(a.pos - b.pos, c.pos - b.pos);
            const real proj_pdf = pdf / std::abs(cos(b.nor, a.pos - b.pos));
            a3 = { &a.pdf_bwd, proj_pdf };
        }
        AGZ_UNACCESSED(a3);
    
        const real connected_G = G(b.pos, c.pos, b.nor, c.nor);
    
        return weight_common(connected_path, connected_G);
    }

} // namespace anonymous

CameraSubpath build_camera_subpath(
    int max_cam_vtx_cnt,
    int px, int py, const Scene &scene, const Vec2 &full_res,
    Sampler &sampler, Arena &arena, BDPTVertex *subpath_space)
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
    const auto cam_pdf = camera->pdf_we(
        cam_sam.pos_on_cam, cam_sam.pos_to_out);

    // fill the first vertex

    subpath_space[0].pos           = cam_sam.pos_on_cam;
    subpath_space[0].nor           = cam_sam.nor_on_cam;
    subpath_space[0].accu_bsdf     = Spectrum(1);
    subpath_space[0].accu_proj_pdf = cam_pdf.pdf_pos;
    subpath_space[0].pdf_fwd       = cam_pdf.pdf_pos;
    subpath_space[0].pdf_bwd       = 0;
    subpath_space[0].G_with_last   = 1;

    // g-buffer pixel
    CameraSubpath::GPixel gpixel;

    // proj pdf to next vertex
    real proj_pdf = cam_pdf.pdf_dir
                  / std::abs(cos(cam_sam.nor_on_cam, cam_sam.pos_to_out));

    // accumulated proj pdf
    real accu_proj_pdf = cam_pdf.pdf_pos * proj_pdf;

    // accumulated bsdf
    Spectrum accu_bsdf = camera->eval_we(
        cam_sam.pos_on_cam, cam_sam.pos_to_out).we;

    // position of last vertex
    Vec3 last_pos = cam_sam.pos_on_cam;

    // normal at last vertex
    Vec3 last_nor = cam_sam.nor_on_cam;

    // current tracing ray
    Ray ray(cam_sam.pos_on_cam, cam_sam.pos_to_out);

    int cam_vtx_cnt = 1;
    while(cam_vtx_cnt < max_cam_vtx_cnt)
    {
        // find next vertex

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
        {
            // create 'no entity' vertex

            BDPTVertex end_vertex;
            end_vertex.pos           = ray.d;
            end_vertex.accu_bsdf     = accu_bsdf;
            end_vertex.accu_proj_pdf = accu_proj_pdf;
            end_vertex.pdf_fwd       = proj_pdf;
            subpath_space[cam_vtx_cnt++] = end_vertex;

            break;
        }

        const ShadingPoint shd = inct.material->shade(inct, arena);

        // fill g-buffer

        if(cam_vtx_cnt == 1)
        {
            gpixel.albedo = shd.bsdf->albedo();
            gpixel.normal = shd.shading_normal;
            gpixel.denoise = inct.entity->get_no_denoise_flag() ?
                             real(0) : real(1);
        }

        // add new vertex

        BDPTVertex vertex;
        vertex.pos           = inct.pos;
        vertex.nor           = inct.geometry_coord.z;
        vertex.accu_bsdf     = accu_bsdf;
        vertex.accu_proj_pdf = accu_proj_pdf;
        vertex.pdf_fwd       = proj_pdf;
        vertex.G_with_last   = G(
            inct.pos, last_pos, inct.geometry_coord.z, last_nor);
        vertex.entity        = inct.entity;
        vertex.uv            = inct.uv;
        vertex.bsdf          = shd.bsdf;
        vertex.is_delta      = shd.bsdf->is_delta();

        last_pos = vertex.pos;
        last_nor = vertex.nor;

        subpath_space[cam_vtx_cnt++] = vertex;

        // sample bsdf

        const auto bsdf_sample = shd.bsdf->sample_all(
            inct.wr, TransMode::Radiance, sampler.sample3());
        if(!bsdf_sample.f)
            break;

        proj_pdf = bsdf_sample.pdf
                 / std::abs(cos(vertex.nor, bsdf_sample.dir));

        accu_bsdf     *= bsdf_sample.f;
        accu_proj_pdf *= proj_pdf;

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    // fill backward proj pdf

    const int max_bwd_index = subpath_space[cam_vtx_cnt - 1].is_entity() ?
                              cam_vtx_cnt - 3 : cam_vtx_cnt - 4;

    for(int i = 0; i <= max_bwd_index; ++i)
    {
        BDPTVertex &a = subpath_space[i];
        const BDPTVertex &b = subpath_space[i + 1];
        const BDPTVertex &c = subpath_space[i + 2];

        // store pdf(c -> b -> a) into a.bwd_pdf

        const real pdf = b.bsdf->pdf_all(c.pos - b.pos, a.pos - b.pos);
        a.pdf_bwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    if(!subpath_space[cam_vtx_cnt - 1].is_entity() && cam_vtx_cnt >= 3)
    {
        // .., a, b, c

        BDPTVertex &a = subpath_space[cam_vtx_cnt - 3];
        const BDPTVertex &b = subpath_space[cam_vtx_cnt - 2];
        const BDPTVertex &c = subpath_space[cam_vtx_cnt - 1];

        const real pdf = b.bsdf->pdf_all(c.pos, a.pos - b.pos);
        a.pdf_bwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    return CameraSubpath{
        pixel_coord.x, pixel_coord.y,
        cam_vtx_cnt, subpath_space, gpixel
    };
}

LightSubpath build_light_subpath(
    int max_lht_vtx_cnt, const Scene &scene,
    Sampler &sampler, Arena &arena, BDPTVertex *subpath_space)
{
    assert(subpath_space);

    // sample light emission

    const auto [light, select_light_pdf] = scene.sample_light(sampler.sample1());
    assert(light);
    const auto light_emit = light->sample_emit(sampler.sample5());

    // fill the first vertex

    subpath_space[0].pos           = light_emit.pos;
    subpath_space[0].nor           = light_emit.nor;
    subpath_space[0].accu_bsdf     = Spectrum(1);
    subpath_space[0].accu_proj_pdf = select_light_pdf * light_emit.pdf_pos;
    subpath_space[0].pdf_fwd       = 0;
    subpath_space[0].pdf_bwd       = select_light_pdf * light_emit.pdf_pos;
    subpath_space[0].G_with_last   = 1;

    // proj pdf to next vertex
    real proj_pdf = light_emit.pdf_dir
                  / std::abs(cos(light_emit.nor, light_emit.dir));

    // accumulated proj pdf
    real accu_proj_pdf = select_light_pdf * light_emit.pdf_pos * proj_pdf;

    // accumulated bsdf
    Spectrum accu_bsdf = light_emit.radiance;

    // position of last vertex
    Vec3 last_pos = light_emit.pos;

    // normal at last vertex
    Vec3 last_nor = light_emit.nor;

    // current tracing ray
    Ray ray(light_emit.pos, light_emit.dir, EPS);

    int lht_vtx_cnt = 1;
    while(lht_vtx_cnt < max_lht_vtx_cnt)
    {
        // find next vertex

        EntityIntersection inct;
        if(!scene.closest_intersection(ray, &inct))
            break;

        const ShadingPoint shd = inct.material->shade(inct, arena);

        // add new vertex

        BDPTVertex vertex;
        vertex.pos           = inct.pos;
        vertex.nor           = inct.geometry_coord.z;
        vertex.accu_bsdf     = accu_bsdf;
        vertex.accu_proj_pdf = accu_proj_pdf;
        vertex.pdf_fwd       = 0;
        vertex.pdf_bwd       = proj_pdf;
        vertex.G_with_last   = G(
            inct.pos, last_pos, inct.geometry_coord.z, last_nor);
        vertex.entity        = inct.entity;
        vertex.uv            = inct.uv;
        vertex.bsdf          = shd.bsdf;
        vertex.is_delta      = shd.bsdf->is_delta();

        last_pos = vertex.pos;
        last_nor = vertex.nor;

        subpath_space[lht_vtx_cnt++] = vertex;

        // sample bsdf

        const auto bsdf_sample = shd.bsdf->sample_all(
            inct.wr, TransMode::Importance, sampler.sample3());
        if(!bsdf_sample.f)
            break;

        proj_pdf = bsdf_sample.pdf
                 / std::abs(cos(vertex.nor, bsdf_sample.dir));

        accu_bsdf     *= bsdf_sample.f;
        accu_proj_pdf *= proj_pdf;

        ray = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
    }

    // fill forward proj pdf

    const int max_fwd_index = lht_vtx_cnt - 3;

    for(int i = 1; i <= max_fwd_index; ++i)
    {
        BDPTVertex &a = subpath_space[i];
        const BDPTVertex &b = subpath_space[i + 1];
        const BDPTVertex &c = subpath_space[i + 2];

        // store pdf(c -> b -> a) into a.pdf_fwd

        const real pdf = b.bsdf->pdf_all(c.pos - b.pos, a.pos - b.pos);
        a.pdf_fwd = pdf / std::abs(cos(b.nor, a.pos - b.pos));
    }

    // fill subpath_space[0].pdf_fwd

    if(lht_vtx_cnt >= 3)
    {
        BDPTVertex &a = subpath_space[0];
        const BDPTVertex &b = subpath_space[1];
        const BDPTVertex &c = subpath_space[2];

        const real pdf = b.bsdf->pdf_all(c.pos - b.pos, -light_emit.dir);
        a.pdf_fwd = pdf / std::abs(cos(b.nor, light_emit.dir));
    }

    return LightSubpath{
        lht_vtx_cnt, subpath_space, select_light_pdf, light
    };
}

Spectrum eval_connected_subpath(
    const ConnectedPath &connected_path, Vec2 *pixel_coord)
{
    const int s = connected_path.s;
    const int t = connected_path.t;
    const int path_vtx_cnt = s + t;

    // an valid path contains at least 2 vertices

    if(path_vtx_cnt < 2)
        return {};

    // use native path tracing for paths with only 2 vertices

    if(path_vtx_cnt == 2)
        return s == 2 ? contrib_s2_t0_path(connected_path) : Spectrum();

    if(s == 1)
    {
        assert(t >= 2);
        return contrib_s1_tx_path(connected_path, pixel_coord);
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

std::optional<BDPTPixel> trace_bdpt(
    const BDPTParams &params, const Scene &scene,
    int px, int py, const Vec2 &full_res,
    Sampler &sampler, Arena &arena,
    BDPTVertex *cam_subpath_space,
    BDPTVertex *lht_subpath_space,
    FilmFilterApplier::FilmGridView<Spectrum> *particle_film)
{
    if(scene.lights().empty())
        return std::nullopt;

    const CameraSubpath cam_subpath = build_camera_subpath(
        params.max_cam_vtx_cnt, px, py, scene, full_res,
        sampler, arena, cam_subpath_space);

    const LightSubpath lht_subpath = build_light_subpath(
        params.max_lht_vtx_cnt, scene, sampler, arena, lht_subpath_space);

    const Rect2i particle_sample_pixels = particle_film->sample_pixels();
    const Vec2 particle_film_full_res = {
        real(particle_sample_pixels.high.x - particle_sample_pixels.low.x + 1),
        real(particle_sample_pixels.high.y - particle_sample_pixels.low.y + 1)
    };

    const Rect2i sample_pixels = particle_film->sample_pixels();
    const Rect2 sample_pixel_bound = {
        { real(sample_pixels.low.x), real(sample_pixels.low.y) },
        {
            real(sample_pixels.high.x + 1),
            real(sample_pixels.high.y + 1)
        }
    };

    ConnectedPath connected_path = {
        scene, lht_subpath.light, scene.get_camera(),
        lht_subpath.select_light_pdf,
        cam_subpath.subpath, 0,
        lht_subpath.subpath, 0,
        sampler, sample_pixel_bound, particle_film_full_res
    };

    connected_path.s = 1;
    for(int t = 0; t <= lht_subpath.vtx_cnt; ++t)
    {
        connected_path.t = t;

        Vec2 pixel_coord;
        const Spectrum rad = eval_connected_subpath(
            connected_path, &pixel_coord);
        if(!rad.is_black())
            particle_film->apply(pixel_coord.x, pixel_coord.y, rad);
    }

    Spectrum radiance;
    for(int s = 2; s <= cam_subpath.vtx_cnt; ++s)
    {
        for(int t = 0; t <= lht_subpath.vtx_cnt; ++t)
        {
            connected_path.s = s;
            connected_path.t = t;
            radiance += eval_connected_subpath(connected_path, nullptr);
        }
    }

    if(radiance.is_finite())
    {
        return BDPTPixel{
            cam_subpath.pixel_x, cam_subpath.pixel_y,
            Pixel{
                {
                    cam_subpath.gpixel.albedo,
                    cam_subpath.gpixel.normal,
                    cam_subpath.gpixel.denoise
                },
                radiance
            }
        };
    }

    return std::nullopt;
}

} // namespace bdpt

AGZ_TRACER_RENDER_END
