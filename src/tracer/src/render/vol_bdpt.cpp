#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/camera.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/scene.h>

#include <agz/tracer/render/vol_bdpt.h>

AGZ_TRACER_RENDER_BEGIN

namespace vol_bdpt
{

namespace
{
    using TempAssign = misc::scope_assignment_t<real>;

    real pdf_sa_to_area(
        real pdf_sa, const Vec3 &this_pos, const Vertex &dst_vtx) noexcept
    {
        switch(dst_vtx.type)
        {
        case VertexType::Camera:
            {
                const Vec3 d = dst_vtx.camera.pos - this_pos;
                const real dst_cos = std::abs(cos(dst_vtx.camera.nor, d));
                const real dist2 = d.length_square();
                return pdf_sa * dst_cos / dist2;
            }
        case VertexType::EnvLight:
            return pdf_sa;
        case VertexType::Medium:
                return pdf_sa / distance2(dst_vtx.medium.pos, this_pos);
        case VertexType::AreaLight:
            {
                const Vec3 d       = dst_vtx.area_light.pos - this_pos;
                const real dst_cos = std::abs(cos(dst_vtx.area_light.nor, d));
                const real dist2   = d.length_square();
                return pdf_sa * dst_cos / dist2;
            }
        default:
            {
                const Vec3 d = dst_vtx.surface.pos - this_pos;
                const real dst_cos = std::abs(cos(dst_vtx.surface.nor, d));
                const real dist2 = d.length_square();
                return pdf_sa * dst_cos / dist2;
            }
        }

        // misc::unreachable()
    }

    const Medium *get_vertex_medium(const Vertex &a, const Vertex &b) noexcept
    {
        assert(a.is_scattering_type() || b.is_scattering_type());

        if(a.type == VertexType::Medium)
            return a.medium.med;

        if(a.type == VertexType::Surface)
        {
            switch(b.type)
            {
            case VertexType::Camera:
                return a.surface.medium(b.camera.pos - a.surface.pos);
            case VertexType::AreaLight:
                return a.surface.medium(b.area_light.pos - a.surface.pos);
            case VertexType::EnvLight:
                return a.surface.medium(-b.env_light.light_to_out);
            case VertexType::Medium:
                return b.medium.med;
            case VertexType::Surface:
                return a.surface.medium(b.surface.pos - a.surface.pos);
            }
        }

        assert(b.is_scattering_type());

        return get_vertex_medium(b, a);
    }

    const BSDF *get_scatter_bsdf(const Vertex &v) noexcept
    {
        assert(v.is_scattering_type());
        if(v.type == VertexType::Surface)
            return v.surface.bsdf;
        return v.medium.phase;
    }

    Vec3 get_scatter_wr(const Vertex &v) noexcept
    {
        assert(v.is_scattering_type());
        if(v.type == VertexType::Surface)
            return v.surface.wr;
        return v.medium.wr;
    }

    Vec3 get_scatter_pos(const Vertex &v) noexcept
    {
        assert(v.is_scattering_type());
        if(v.type == VertexType::Surface)
            return v.surface.pos;
        return v.medium.pos;
    }

    real pdf_to(const Vertex &scattering_vtx, const Vertex &to) noexcept
    {
        const Vec3 from_pos = get_scatter_pos(scattering_vtx);

        Vec3 to_dir;
        switch(to.type)
        {
        case VertexType::Camera:
            to_dir = to.camera.pos - from_pos;
            break;
        case VertexType::Medium:
            to_dir = to.medium.pos - from_pos;
            break;
        case VertexType::AreaLight:
            to_dir = to.area_light.pos - from_pos;
            break;
        case VertexType::Surface:
            to_dir = to.surface.pos - from_pos;
            break;
        case VertexType::EnvLight:
            to_dir = -to.env_light.light_to_out;
            break;
        }

        const real pdf_sa = get_scatter_bsdf(scattering_vtx)->pdf_all(
            to_dir, get_scatter_wr(scattering_vtx));

        return pdf_sa_to_area(pdf_sa, from_pos, to);
    }

    real z2o(real x) noexcept
    {
        return (!math::is_finite(x) || x <= 0) ? 1 : x;
    }

    real mis_weight_common(
        const Vertex *C, int s,
        const Vertex *L, int t,
        real select_light_pdf, const Light *light,
        real init_cur = 1)
    {
        assert(s >= 1 && s + t >= 3);

        real sum_pdf = 1;
        real cur_pdf = init_cur;

        // ===== process light subpath =====

        //    [..., a] <-> [b, c, ...]
        // => [..., a, b] <-> [c, ...]

        for(int i = t - 1; i >= 2; --i)
        {
            const real mul = z2o(L[i].pdf_fwd);
            const real div = z2o(L[i].pdf_bwd);

            cur_pdf *= mul / div;

            // note that L[t - 1].is_delta shouldn't be considered
            // however, contrib is always 0 if L[t - 1].is_delta == true,
            // so we don't need to handle it separately

            if(!L[i].is_delta && !L[i - 1].is_delta)
                sum_pdf += cur_pdf;
        }

        //    [..., a] <-> [b, c]
        // => [..., a, b] <-> [c]

        if(t >= 2)
        {
            const real mul = z2o(L[1].pdf_fwd);
            const real div = z2o(L[1].pdf_bwd);

            cur_pdf *= mul / div;

            if(!L[1].is_delta && !L[0].is_delta)
            {
                // handle pdf of Light::Sample
                // in sx_t1 case we use Light::Sample to sample the light
                // instead of connect two subpaths, which means its pdf needs
                // special consideration

                assert(L[1].is_scattering_type());
                const Vec3 L1_pos = L[1].type == VertexType::Surface ?
                                                 L[1].surface.pos :
                                                 L[1].medium.pos;

                real pdf_li;
                if(L[0].type == VertexType::Surface)
                {
                    assert(light->is_area());
                    const AreaLight *area_light = light->as_area();

                    pdf_li = area_light->pdf(
                        L1_pos, L[0].surface.pos, L[0].surface.nor);
                }
                else if(L[0].type == VertexType::AreaLight)
                {
                    assert(light->is_area());
                    const AreaLight *area_light = light->as_area();

                    pdf_li = area_light->pdf(
                        L1_pos, L[0].area_light.pos, L[0].area_light.nor);
                }
                else
                {
                    assert(L[0].type == VertexType::EnvLight);
                    assert(!light->is_area());
                    const EnvirLight *env_light = light->as_envir();

                    pdf_li = env_light->pdf(
                        L1_pos, -L[0].env_light.light_to_out);
                }

                pdf_li *= select_light_pdf;
                pdf_li = pdf_sa_to_area(pdf_li, L1_pos, L[0]);

                sum_pdf += cur_pdf * z2o(pdf_li) / z2o(L[0].pdf_bwd);
            }
        }

        // light beg

        if(t >= 1)
        {
            const real mul = z2o(L[0].pdf_fwd);
            const real div = z2o(L[0].pdf_bwd);

            cur_pdf *= mul / div;

            if(!L[0].is_delta)
                sum_pdf += cur_pdf;
        }

        // ===== process camera subpath =====

        cur_pdf = init_cur;

        // camera end

        // if t == 0, then we need to handle special case of Light::Sample
        // otherwise we simply move C[s - 1] to light subpath
        if(t == 0)
        {
            //    [..., a, b] <-> []
            // => [..., a] <-> [b]

            assert(s >= 3);
            const Vertex &a = C[s - 2];
            const Vertex &b = C[s - 1];

            const real mul = z2o(b.pdf_bwd);
            const real div = z2o(b.pdf_fwd);

            cur_pdf *= mul / div;

            if(!a.is_delta)
            {
                // b is sampled with Light::Sample and needs special process

                const Vec3 a_pos = a.type == VertexType::Surface ?
                                   a.surface.pos :
                                   a.medium.pos;

                assert(b.type == VertexType::Surface ||
                       b.type == VertexType::EnvLight);

                real pdf_li;
                if(b.type == VertexType::Surface)
                {
                    assert(light->is_area());
                    const AreaLight *area_light = light->as_area();

                    pdf_li = area_light->pdf(
                        a_pos, b.surface.pos, b.surface.nor);
                }
                else
                {
                    assert(!light->is_area());
                    const EnvirLight *envir_light = light->as_envir();

                    pdf_li = envir_light->pdf(
                        a_pos, -b.env_light.light_to_out);
                }

                pdf_li *= select_light_pdf;
                pdf_li = pdf_sa_to_area(pdf_li, a_pos, b);

                sum_pdf += cur_pdf * z2o(pdf_li) / z2o(b.pdf_bwd);
            }
        }
        else if(s >= 2)
        {
            const real mul = z2o(C[s - 1].pdf_bwd);
            const real div = z2o(C[s - 1].pdf_fwd);

            cur_pdf *= mul / div;

            if(!C[s - 2].is_delta)
                sum_pdf += cur_pdf;
        }

        // camera vertices

        for(int i = s - 2; i >= 1; --i)
        {
            const real mul = z2o(C[i].pdf_bwd);
            const real div = z2o(C[i].pdf_fwd);

            cur_pdf *= mul / div;

            if(!C[i].is_delta && !C[i - 1].is_delta)
                sum_pdf += cur_pdf;
        }

        return 1 / sum_pdf;
    }
}

CameraSubpath build_camera_subpath(
    int max_vertex_count, const Ray &ray,
    const Scene &scene, Sampler &sampler,
    Arena &arena, Vertex *vertex_space)
{
    assert(max_vertex_count >= 1);

    CameraSubpath subpath;

    // initial vertex

    auto camera = scene.get_camera();

    const auto cam_we  = camera->eval_we(ray.o, ray.d);
    const auto cam_pdf = camera->pdf_we(ray.o, ray.d);

    auto &cam_vtx = vertex_space[0];
    cam_vtx.type       = VertexType::Camera;
    cam_vtx.accu_coef  = Spectrum(1 / cam_pdf.pdf_pos);
    cam_vtx.pdf_fwd    = cam_pdf.pdf_pos;
    cam_vtx.is_delta   = false;
    cam_vtx.camera.pos = ray.o;
    cam_vtx.camera.nor = cam_we.nor_on_cam;

    Spectrum accu_coef = cam_we.we / (cam_pdf.pdf_pos * cam_pdf.pdf_dir);
    real pdf_fwd       = cam_pdf.pdf_dir;
    Ray r              = ray;

    accu_coef *= std::abs(cos(cam_we.nor_on_cam, ray.d));

    int vertex_count = 1;
    while(vertex_count < max_vertex_count)
    {
        // find next entity vertex

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
        {
            auto env = scene.envir_light();
            if(!env)
                break;

            auto &new_vtx = vertex_space[vertex_count++];
            new_vtx.type                   = VertexType::EnvLight;
            new_vtx.accu_coef              = accu_coef;
            new_vtx.pdf_fwd                = pdf_fwd;
            new_vtx.is_delta               = false;
            new_vtx.env_light.light_to_out = -r.d;

            break;
        }

        // g-pixel

        ShadingPoint shd;
        if(vertex_count == 1)
        {
            shd = inct.material->shade(inct, arena);

            subpath.g_albedo  = shd.bsdf->albedo();
            subpath.g_normal  = shd.shading_normal;
            subpath.g_denoise = real(inct.entity->get_no_denoise_flag() ? 0 : 1);
        }

        // sample medium

        auto medium = inct.medium(inct.wr);
        const auto med_sam = medium->sample_scattering(
            r.o, inct.pos, sampler, arena);
        accu_coef *= med_sam.throughput;

        if(med_sam.is_scattering_happened())
        {
            // add medium vertex

            const auto &sp = med_sam.scattering_point;

            auto &new_vtx = vertex_space[vertex_count++];
            new_vtx.type         = VertexType::Medium;
            new_vtx.accu_coef    = accu_coef;
            new_vtx.pdf_fwd      = pdf_fwd / distance2(sp.pos, r.o);
            new_vtx.is_delta     = false;
            new_vtx.medium.pos   = sp.pos;
            new_vtx.medium.wr    = inct.wr;
            new_vtx.medium.med   = medium;
            new_vtx.medium.phase = med_sam.phase_function;

            // sample the phase function

            const auto phase_sample = med_sam.phase_function->sample_all(
                sp.wr, TransMode::Radiance, sampler.sample3());
            assert(!phase_sample.invalid());

            // update pdf_bwd of last vertex according to phase sample

            const real pdf_bwd = med_sam.phase_function->pdf_all(
                inct.wr, phase_sample.dir);
            auto &last_vtx = vertex_space[vertex_count - 2];
            last_vtx.pdf_bwd = pdf_sa_to_area(pdf_bwd, sp.pos, last_vtx);

            // update ray payload

            accu_coef *= phase_sample.f / phase_sample.pdf;
            pdf_fwd    = phase_sample.pdf;
            r          = Ray(sp.pos, phase_sample.dir);
        }
        else
        {
            // add surface vertex

            if(!shd.bsdf)
                shd = inct.material->shade(inct, arena);

            const real cos_inct = std::abs(cos(inct.geometry_coord.z, inct.wr));
            const real dist2 = distance2(r.o, inct.pos);
            const real pdf_area = pdf_fwd * cos_inct / dist2;

            auto &new_vtx = vertex_space[vertex_count++];
            new_vtx.type            = VertexType::Surface;
            new_vtx.accu_coef       = accu_coef;
            new_vtx.pdf_fwd         = pdf_area;
            new_vtx.is_delta        = shd.bsdf->is_delta();
            new_vtx.surface.pos     = inct.pos;
            new_vtx.surface.nor     = inct.geometry_coord.z;
            new_vtx.surface.uv      = inct.uv;
            new_vtx.surface.wr      = inct.wr;
            new_vtx.surface.med_out = inct.medium_out;
            new_vtx.surface.med_in  = inct.medium_in;
            new_vtx.surface.bsdf    = shd.bsdf;
            new_vtx.surface.entity  = inct.entity;

            // sample bsdf

            const auto bsdf_sample = shd.bsdf->sample_all(
                inct.wr, TransMode::Radiance, sampler.sample3());
            if(bsdf_sample.invalid())
                break;

            // update pdf_bwd of last vertex according to bsdf sample

            const real pdf_bwd = shd.bsdf->pdf_all(
                inct.wr, bsdf_sample.dir);
            auto &last_vtx = vertex_space[vertex_count - 2];
            last_vtx.pdf_bwd = pdf_sa_to_area(pdf_bwd, inct.pos, last_vtx);

            // update ray payload

            const real abscos = std::abs(cos(
                inct.geometry_coord.z, bsdf_sample.dir));

            accu_coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;
            pdf_fwd    = bsdf_sample.pdf;
            r          = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }
    }

    subpath.vertex_count = vertex_count;
    subpath.vertices     = vertex_space;
    return subpath;
}

LightSubpath build_light_subpath(
    int max_vertex_count,
    const SceneSampleLightResult &select_light,
    const Scene &scene, Sampler &sampler,
    Arena &arena, Vertex *vertex_space)
{
    assert(max_vertex_count >= 1);

    const Light *light = select_light.light;
    const real select_light_pdf = select_light.pdf;

    // sample emitter

    const auto light_emit = light->sample_emit(sampler.sample5());
    if(!light_emit.radiance)
    {
        LightSubpath subpath;
        subpath.vertex_count = 0;
        return subpath;
    }

    Spectrum accu_coef;
    real pdf_bwd;

    // initial vertex

    auto &init_vtx = vertex_space[0];
    if(light->is_area())
    {
        const real init_pdf = light_emit.pdf_pos * select_light_pdf;

        init_vtx .type             = VertexType::AreaLight;
        init_vtx .accu_coef        = Spectrum(1 / init_pdf);
        init_vtx .pdf_bwd          = light_emit.pdf_pos * select_light_pdf;
        init_vtx .is_delta         = false;
        init_vtx .area_light.pos   = light_emit.pos;
        init_vtx .area_light.nor   = light_emit.nor;
        init_vtx .area_light.light = light->as_area();

        const real emit_cos = std::abs(cos(light_emit.nor, light_emit.dir));
        accu_coef = light_emit.radiance * emit_cos
                  / (select_light_pdf * light_emit.pdf_pos * light_emit.pdf_dir);
        pdf_bwd = light_emit.pdf_dir;
    }
    else
    {
        const real init_pdf = light_emit.pdf_dir * select_light_pdf;

        init_vtx.type                   = VertexType::EnvLight;
        init_vtx.accu_coef              = Spectrum(1 / init_pdf);
        init_vtx.pdf_bwd                = light_emit.pdf_dir * select_light_pdf;
        init_vtx.is_delta               = false;
        init_vtx.env_light.light_to_out = light_emit.dir;

        accu_coef = light_emit.radiance / (init_pdf * light_emit.pdf_pos);
        pdf_bwd   = light_emit.pdf_pos; // wrong value. will be corrected
    }

    Ray r(light_emit.pos, light_emit.dir, EPS());

    int vertex_count = 1;
    while(vertex_count < max_vertex_count)
    {
        // find the closest intersection

        EntityIntersection inct;
        if(!scene.closest_intersection(r, &inct))
            break;

        // sample medium

        const auto medium = inct.medium(inct.wr);
        const auto med_sam = medium->sample_scattering(
            r.o, inct.pos, sampler, arena);
        accu_coef *= med_sam.throughput;

        if(med_sam.is_scattering_happened())
        {
            // add medium vertex

            auto &sp = med_sam.scattering_point;

            auto &new_vtx = vertex_space[vertex_count++];
            new_vtx.type         = VertexType::Medium;
            new_vtx.accu_coef    = accu_coef;
            new_vtx.pdf_bwd      = pdf_bwd / distance2(r.o, sp.pos);
            new_vtx.is_delta     = false;
            new_vtx.medium.pos   = sp.pos;
            new_vtx.medium.wr    = inct.wr;
            new_vtx.medium.med   = medium;
            new_vtx.medium.phase = med_sam.phase_function;

            // sample phase function
            // sample the phase function

            const auto phase_sample = med_sam.phase_function->sample_all(
                sp.wr, TransMode::Importance, sampler.sample3());
            assert(!phase_sample.invalid());

            // update pdf_bwd of last vertex according to phase sample

            const real pdf_fwd = med_sam.phase_function->pdf_all(
                inct.wr, phase_sample.dir);
            auto &last_vtx = vertex_space[vertex_count - 2];
            last_vtx.pdf_fwd = pdf_sa_to_area(pdf_fwd, sp.pos, last_vtx);

            // update ray payload

            accu_coef *= phase_sample.f / phase_sample.pdf;
            pdf_bwd    = phase_sample.pdf;
            r          = Ray(sp.pos, phase_sample.dir);
        }
        else
        {
            // add surface vertex

            const auto shd = inct.material->shade(inct, arena);

            const real cos_inct = std::abs(cos(inct.geometry_coord.z, inct.wr));
            const real dist2 = distance2(r.o, inct.pos);
            const real pdf_area = pdf_bwd * cos_inct / dist2;

            auto &new_vtx = vertex_space[vertex_count++];
            new_vtx.type            = VertexType::Surface;
            new_vtx.accu_coef       = accu_coef;
            new_vtx.pdf_bwd         = pdf_area;
            new_vtx.is_delta        = shd.bsdf->is_delta();
            new_vtx.surface.pos     = inct.pos;
            new_vtx.surface.nor     = inct.geometry_coord.z;
            new_vtx.surface.uv      = inct.uv;
            new_vtx.surface.wr      = inct.wr;
            new_vtx.surface.med_out = inct.medium_out;
            new_vtx.surface.med_in  = inct.medium_in;
            new_vtx.surface.bsdf    = shd.bsdf;
            new_vtx.surface.entity  = inct.entity;

            // sample bsdf

            const auto bsdf_sample = shd.bsdf->sample_all(
                inct.wr, TransMode::Importance, sampler.sample3());
            if(bsdf_sample.invalid())
                break;

            // update pdf_bwd of last vertex according to bsdf sample

            const real pdf_fwd = shd.bsdf->pdf_all(
                inct.wr, bsdf_sample.dir);
            auto &last_vtx = vertex_space[vertex_count - 2];
            last_vtx.pdf_fwd = pdf_sa_to_area(pdf_fwd, inct.pos, last_vtx);

            // update ray payload

            const real abscos = std::abs(cos(
                inct.geometry_coord.z, bsdf_sample.dir));
            accu_coef *= bsdf_sample.f * abscos / bsdf_sample.pdf;
            pdf_bwd    = bsdf_sample.pdf;
            r          = Ray(inct.eps_offset(bsdf_sample.dir), bsdf_sample.dir);
        }
    }

    if(!light->is_area())
    {
        if(vertex_count >= 2)
            vertex_space[1].pdf_bwd = light_emit.pdf_pos;
    }

    LightSubpath subpath;
    subpath.vertex_count = vertex_count;
    subpath.vertices     = vertex_space;
    return subpath;
}

Spectrum contrib_s2_t0(
    const Scene &scene,
    const Vertex *camera_subpath)
{
    const Vertex &cam_beg = camera_subpath[0];
    const Vertex &cam_end = camera_subpath[1];

    // envir light

    if(cam_end.type == VertexType::EnvLight)
    {
        const Spectrum light_radiance = scene.envir_light()->radiance(
            cam_beg.camera.pos, -cam_end.env_light.light_to_out);

        return cam_end.accu_coef * light_radiance;
    }

    // medium

    if(cam_end.type == VertexType::Medium)
        return {};

    // area light

    assert(cam_end.type == VertexType::Surface);
    auto light = cam_end.surface.entity->as_light();
    if(!light)
        return {};

    const Spectrum light_radiance = light->radiance(
        cam_end.surface.pos, cam_end.surface.nor, cam_end.surface.uv,
        cam_beg.camera.pos - cam_end.surface.pos);

    return cam_end.accu_coef * light_radiance;
}

Spectrum unweighted_contrib_sx_t0(
    const Scene &scene, const Vertex *camera_subpath, int s)
{
    assert(s >= 3);

    // path: ..., a, b

    const Vertex &a = camera_subpath[s - 2];
    const Vertex &b = camera_subpath[s - 1];

    // get position of a

    assert(a.type == VertexType::Surface || a.type == VertexType::Medium);

    const Vec3 a_pos = a.type == VertexType::Surface ? a.surface.pos
                                                     : a.medium.pos;
    
    // envir light

    if(b.type == VertexType::EnvLight)
    {
        const Spectrum light_radiance = scene.envir_light()->radiance(
            a_pos, -b.env_light.light_to_out);

        return b.accu_coef * light_radiance;
    }

    // medium

    if(b.type == VertexType::Medium)
        return {};

    // area light

    assert(b.type == VertexType::Surface);
    auto light = b.surface.entity->as_light();
    if(!light)
        return {};

    const Spectrum light_radiance = light->radiance(
        b.surface.pos, b.surface.nor, b.surface.uv,
        a_pos - b.surface.pos);

    return b.accu_coef * light_radiance;
}

Spectrum unweighted_contrib_sx_t1(
    const Scene &scene,
    const Vertex *camera_subpath, int s,
    Sampler &sampler,
    SceneSampleLightResult &select_light,
    LightSampleResult &light_sample)
{
    assert(s >= 2);

    auto &cam_end = camera_subpath[s - 1];
    
    if(cam_end.type == VertexType::EnvLight)
        return {};

    assert(cam_end.is_scattering_type());

    // select a light source

    select_light = scene.sample_light(sampler.sample1());
    if(!select_light.light)
        return {};

    // get cam_end.pos

    const Vec3 cam_end_pos = cam_end.type == VertexType::Surface ?
                             cam_end.surface.pos :
                             cam_end.medium.pos;

    // sample dir to light source from cam_end_pos

    light_sample = select_light.light->sample(cam_end_pos, sampler.sample5());
    if(!light_sample.radiance)
        return {};

    // visibility test

    if(!scene.visible(cam_end_pos, light_sample.pos))
        return {};

    // eval bsdf

    const auto bsdf = get_scatter_bsdf(cam_end);
    Spectrum bsdf_f = bsdf->eval_all(
        light_sample.ref_to_light(),
        get_scatter_wr(cam_end),
        TransMode::Radiance);

    if(!bsdf_f)
        return {};

    // eval abscos

    if(cam_end.type == VertexType::Surface)
    {
        bsdf_f *= std::abs(cos(
            cam_end.surface.nor, light_sample.ref_to_light()));
    }

    // eval tr

    const auto medium = cam_end.type == VertexType::Surface ?
                        cam_end.surface.medium(light_sample.ref_to_light()) :
                        cam_end.medium.med;;
    const Spectrum tr = medium->tr(cam_end_pos, light_sample.pos, sampler);

    // conpute contrib

    return cam_end.accu_coef * bsdf_f * tr * light_sample.radiance
         / light_sample.pdf;
}

Spectrum unweighted_contrib_s1_tx(
    const Scene &scene,
    const Vertex *camera_subpath,
    const Vertex *light_subpath, int t,
    Sampler &sampler,
    const Rect2 &sample_pixel_bound,
    const Vec2 &full_res,
    Vec2 &pixel_coord)
{
    assert(t >= 2);

    auto &lht_end = light_subpath[t - 1];
    assert(lht_end.is_scattering_type());

    // get light end pos

    const Vec3 lht_end_pos = lht_end.type == VertexType::Surface ?
                             lht_end.surface.pos :
                             lht_end.medium.pos;

    // eval camera

    const Vec3 cam_pos = camera_subpath[0].camera.pos;
    const auto cam_we = scene.get_camera()->eval_we(
        cam_pos, lht_end_pos - cam_pos);

    if(!cam_we.we)
        return {};

    // check pixel coord

    pixel_coord = {
        cam_we.film_coord.x * full_res.x,
        cam_we.film_coord.y * full_res.y
    };

    if(!sample_pixel_bound.contains(pixel_coord))
        return {};

    // check visibility

    if(!scene.visible(lht_end_pos, cam_pos))
        return {};

    // eval bsdf

    const Vec3 lht_to_cam = cam_pos - lht_end_pos;

    const BSDF *bsdf = get_scatter_bsdf(lht_end);
    const Spectrum f = bsdf->eval_all(
        lht_to_cam, get_scatter_wr(lht_end),
        TransMode::Importance);
    if(!f)
        return {};

    // eval G

    real G = 1 / distance2(cam_pos, lht_end_pos);

    if(lht_end.type == VertexType::Surface)
        G *= std::abs(cos(lht_to_cam, lht_end.surface.nor));

    G *= std::abs(cos(lht_to_cam, cam_we.nor_on_cam));

    // eval tr

    const Medium *med = lht_end.type == VertexType::Surface ?
                        lht_end.surface.medium(lht_to_cam) :
                        lht_end.medium.med;

    const Spectrum tr = med->tr(cam_pos, lht_end_pos, sampler);

    // eval contrib

    return cam_we.we * tr * G * f * lht_end.accu_coef
         / camera_subpath[0].pdf_fwd;
}

Spectrum unweighted_contrib_sx_tx(
    const Scene &scene,
    const Vertex *camera_subpath, int s,
    const Vertex *light_subpath, int t,
    Sampler &sampler)
{
    assert(s >= 2 && t >= 2);

    const Vertex &cam_end = camera_subpath[s - 1];
    const Vertex &lht_end = light_subpath[t - 1];

    if(cam_end.type == VertexType::EnvLight)
        return {};

    assert(cam_end.is_scattering_type() && lht_end.is_scattering_type());

    const Vec3 cam_end_pos = get_scatter_pos(cam_end);
    const Vec3 lht_end_pos = get_scatter_pos(lht_end);

    // visibility

    if(!scene.visible(cam_end_pos, lht_end_pos))
        return {};

    // bsdf

    const Vec3 cam_to_lht = lht_end_pos - cam_end_pos;

    const BSDF *cam_end_bsdf = get_scatter_bsdf(cam_end);
    Spectrum cam_bsdf_f = cam_end_bsdf->eval_all(
        cam_to_lht, get_scatter_wr(cam_end), TransMode::Radiance);
    if(!cam_bsdf_f)
        return {};

    const BSDF *lht_end_bsdf = get_scatter_bsdf(lht_end);
    Spectrum lht_bsdf_f = lht_end_bsdf->eval_all(
        -cam_to_lht, get_scatter_wr(lht_end), TransMode::Importance);
    if(!lht_bsdf_f)
        return {};

    // G

    real G = 1 / distance2(cam_end_pos, lht_end_pos);

    if(cam_end.type == VertexType::Surface)
        G *= std::abs(cos(cam_end.surface.nor, cam_to_lht));

    if(lht_end.type == VertexType::Surface)
        G *= std::abs(cos(lht_end.surface.nor, cam_to_lht));

    // tr

    const Medium *medium = get_vertex_medium(cam_end, lht_end);
    const Spectrum tr = medium->tr(cam_end_pos, lht_end_pos, sampler);

    // eval contrib

    return cam_end.accu_coef * cam_bsdf_f
         * G * tr
         * lht_end.accu_coef * lht_bsdf_f;
}

real mis_weight_sx_t0(
    const Scene &scene,
    Vertex *camera_subpath, int s)
{
    // ..., a, b

    assert(s >= 3);

    Vertex &b = camera_subpath[s - 1];
    Vertex &a = camera_subpath[s - 2];

    TempAssign assign_b_pdf_bwd;
    TempAssign assign_a_pdf_bwd;

    AGZ_UNACCESSED(assign_b_pdf_bwd); // for resharper's warnings
    AGZ_UNACCESSED(assign_a_pdf_bwd);

    real select_light_pdf;
    const Light *light;

    if(b.type == VertexType::Surface)
    {
        light = b.surface.entity->as_light();
        if(!light)
            return 0;

        select_light_pdf = scene.light_pdf(light);
        const auto light_pdf = light->emit_pdf(
            b.surface.pos, b.surface.wr, b.surface.nor);

        assign_b_pdf_bwd = {
            &b.pdf_bwd,
            select_light_pdf * light_pdf.pdf_pos
        };

        assign_a_pdf_bwd = {
            &a.pdf_bwd,
            pdf_sa_to_area(light_pdf.pdf_dir, b.surface.pos, a)
        };
    }
    else if(b.type == VertexType::EnvLight)
    {
        auto env = scene.envir_light();
        light = env;
        assert(env);

        select_light_pdf = scene.light_pdf(env);
        const auto light_pdf = env->emit_pdf({}, b.env_light.light_to_out, {});

        assign_b_pdf_bwd = {
            &b.pdf_bwd,
            select_light_pdf * light_pdf.pdf_dir
        };

        assign_a_pdf_bwd = {
            &a.pdf_bwd,
            light_pdf.pdf_pos
        };
    }
    else
        return 0;

    return mis_weight_common(
        camera_subpath, s, nullptr, 0, select_light_pdf, light);
}

real mis_weight_sx_t1(
    Vertex *camera_subpath, int s,
    const SceneSampleLightResult &select_light,
    const LightSampleResult &light_sample)
{
    assert(s >= 2);

    // [..., a, b] <-> [c]

    Vertex &a = camera_subpath[s - 2];
    Vertex &b = camera_subpath[s - 1];

    const Vec3 b_pos = get_scatter_pos(b);

    TempAssign a_pdf_bwd_assign;
    TempAssign b_pdf_bwd_assign;

    AGZ_UNACCESSED(a_pdf_bwd_assign);
    AGZ_UNACCESSED(b_pdf_bwd_assign);

    Vertex light_vertex;
    real init_cur_pdf;

    if(select_light.light->is_area())
    {
        const AreaLight *area_light = select_light.light->as_area();

        const LightEmitPDFResult emit_pdf = area_light->emit_pdf(
            light_sample.pos, b_pos - light_sample.pos, light_sample.nor);

        // light vertex

        light_vertex.type             = VertexType::AreaLight;
        light_vertex.accu_coef        = Spectrum(1);
        light_vertex.is_delta         = false;
        light_vertex.area_light.pos   = light_sample.pos;
        light_vertex.area_light.nor   = light_sample.nor;
        light_vertex.area_light.light = area_light;

        light_vertex.pdf_fwd = pdf_to(b, light_vertex);
        light_vertex.pdf_bwd = select_light.pdf * emit_pdf.pdf_pos;

        // b.pdf_bwd

        const real b_pdf_bwd = pdf_sa_to_area(
            emit_pdf.pdf_dir, light_sample.pos, b);

        b_pdf_bwd_assign = { &b.pdf_bwd, b_pdf_bwd };

        // a.pdf_bwd

        const real a_pdf_bwd_sa = get_scatter_bsdf(b)->pdf_all(
            get_scatter_wr(b), light_sample.pos - b_pos);
        const real a_pdf_bwd = pdf_sa_to_area(a_pdf_bwd_sa, b_pos, a);

        a_pdf_bwd_assign = { &a.pdf_bwd, a_pdf_bwd };

        // initial 'current pdf'
        // for eliminating effect of using Light::Sample

        init_cur_pdf = emit_pdf.pdf_pos /
                       pdf_sa_to_area(light_sample.pdf, b_pos, light_vertex);
    }
    else
    {
        const EnvirLight *env_light = select_light.light->as_envir();

        const LightEmitPDFResult emit_pdf = env_light->emit_pdf(
            light_sample.pos, light_sample.ref_to_light(), light_sample.nor);

        // light vertex

        light_vertex.type                   = VertexType::EnvLight;
        light_vertex.accu_coef              = Spectrum(1);
        light_vertex.is_delta               = false;
        light_vertex.env_light.light_to_out = -light_sample.ref_to_light();

        light_vertex.pdf_fwd = pdf_to(b, light_vertex);
        light_vertex.pdf_bwd = select_light.pdf * emit_pdf.pdf_dir;

        // b.pdf_bwd

        b_pdf_bwd_assign = { &b.pdf_bwd, emit_pdf.pdf_pos };

        // a.pdf_bwd

        const real a_pdf_bwd_sa = get_scatter_bsdf(b)->pdf_all(
            get_scatter_wr(b), light_sample.ref_to_light());
        const real a_pdf_bwd = pdf_sa_to_area(a_pdf_bwd_sa, b_pos, a);

        a_pdf_bwd_assign = { &a.pdf_bwd, a_pdf_bwd };

        // initial 'current pdf'
        // for using Light::Sample

        init_cur_pdf = emit_pdf.pdf_dir /
                       pdf_sa_to_area(light_sample.pdf, b_pos, light_vertex);
    }

    return mis_weight_common(
        camera_subpath, s, &light_vertex, 1,
        select_light.pdf, select_light.light,
        init_cur_pdf);
}

real mis_weight_s1_tx(
    const Scene &scene,
    Vertex *camera_subpath,
    Vertex *light_subpath, int t,
    const SceneSampleLightResult &select_light)
{
    assert(t >= 2);

    // [a] <-> [b, c, ...]

    Vertex &a = camera_subpath[0];
    Vertex &b = light_subpath[t - 1];
    Vertex &c = light_subpath[t - 2];

    const Vec3 b_pos = get_scatter_pos(b);

    auto cam_pdf = scene.get_camera()->pdf_we(
        a.camera.pos, b_pos - a.camera.pos);
    if(!cam_pdf.pdf_pos || !cam_pdf.pdf_dir)
        return 0;

    Vertex camera_vertex;
    camera_vertex.type       = VertexType::Camera;
    camera_vertex.accu_coef  = Spectrum(1);
    camera_vertex.is_delta   = false;
    camera_vertex.pdf_fwd    = cam_pdf.pdf_pos;
    camera_vertex.camera.pos = a.camera.pos;
    camera_vertex.camera.nor = a.camera.nor;
    camera_vertex.pdf_bwd    = pdf_to(b, camera_vertex);

    // b.pdf_fwd

    TempAssign b_pdf_fwd_assign = {
        &b.pdf_fwd,
        pdf_sa_to_area(cam_pdf.pdf_dir, a.camera.pos, b)
    };

    // c.pdf_fwd

    const real c_pdf_fwd_sa = get_scatter_bsdf(b)->pdf_all(
        get_scatter_wr(b), a.camera.pos - b_pos);
    const real c_pdf_fwd = pdf_sa_to_area(c_pdf_fwd_sa, b_pos, c);

    TempAssign c_pdf_fwd_assign = {
        &c.pdf_fwd,
        c_pdf_fwd
    };

    return mis_weight_common(
        &camera_vertex, 1, light_subpath, t,
        select_light.pdf, select_light.light);
}

real mis_weight_sx_tx(
    Vertex *camera_subpath, int s,
    Vertex *light_subpath, int t,
    const SceneSampleLightResult &select_light)
{
    assert(s >= 2 && t >= 2);

    // [..., a, b] <-> [c, d, ...]

    Vertex &a = camera_subpath[s - 2];
    Vertex &b = camera_subpath[s - 1];
    Vertex &c = light_subpath[t - 1];
    Vertex &d = light_subpath[t - 2];

    const Vec3 b_pos = get_scatter_pos(b);
    const Vec3 c_pos = get_scatter_pos(c);

    const BSDF *b_bsdf = get_scatter_bsdf(b);
    const BSDF *c_bsdf = get_scatter_bsdf(c);

    const Vec3 b_to_c = c_pos - b_pos;

    // a.pdf_bwd

    const real a_pdf_bwd_sa = b_bsdf->pdf_all(get_scatter_wr(b), b_to_c);
    const real a_pdf_bwd = pdf_sa_to_area(
        a_pdf_bwd_sa, b_pos, a);

    TempAssign a_pdf_bwd_assign = {
        &a.pdf_bwd, a_pdf_bwd
    };

    // b.pdf_bwd

    const real b_pdf_bwd = pdf_to(c, b);

    TempAssign b_pdf_bwd_assign = {
        &b.pdf_bwd, b_pdf_bwd
    };

    // c.pdf_fwd

    const real c_pdf_fwd = pdf_to(b, c);

    TempAssign c_pdf_bwd_assign = {
        &c.pdf_fwd, c_pdf_fwd
    };

    // d.pdf_fwd

    const real d_pdf_fwd_sa = c_bsdf->pdf_all(get_scatter_wr(c), -b_to_c);
    const real d_pdf_fwd = pdf_sa_to_area(
        d_pdf_fwd_sa, c_pos, d);

    TempAssign d_pdf_fwd_assign = {
        &d.pdf_fwd, d_pdf_fwd
    };

    // eventually...

    return mis_weight_common(
        camera_subpath, s,
        light_subpath, t,
        select_light.pdf, select_light.light);
}

Spectrum weighted_contrib_sx_t0(
    const Scene &scene,
    Vertex *camera_subpath, int s)
{
    const Spectrum unweighted_contrib = unweighted_contrib_sx_t0(
        scene, camera_subpath, s);
    if(!unweighted_contrib.is_finite())
        return Spectrum(REAL_INF);
    if(unweighted_contrib.is_black())
        return {};

    const real weight = mis_weight_sx_t0(scene, camera_subpath, s);

    return weight * unweighted_contrib;
}

Spectrum weighted_contrib_sx_t1(
    const Scene &scene,
    Vertex *camera_subpath, int s,
    Sampler &sampler,
    SceneSampleLightResult &select_light,
    LightSampleResult &light_sample)
{
    const Spectrum unweighted_contrib = unweighted_contrib_sx_t1(
        scene, camera_subpath, s, sampler, select_light, light_sample);
    if(!unweighted_contrib.is_finite())
        return Spectrum(REAL_INF);
    if(unweighted_contrib.is_black())
        return {};

    const real weight = mis_weight_sx_t1(
        camera_subpath, s, select_light, light_sample);

    return weight * unweighted_contrib;
}

Spectrum weighted_contrib_s1_tx(
    const Scene &scene,
    Vertex *camera_subpath,
    Vertex *light_subpath, int t,
    Sampler &sampler,
    const Rect2 &sample_pixel_bound,
    const Vec2 &full_res,
    const SceneSampleLightResult &select_light,
    Vec2 &pixel_coord)
{
    const Spectrum unweighted_contrib = unweighted_contrib_s1_tx(
        scene, camera_subpath, light_subpath, t,
        sampler, sample_pixel_bound, full_res,
        pixel_coord);
    if(!unweighted_contrib.is_finite())
        return Spectrum(REAL_INF);
    if(unweighted_contrib.is_black())
        return {};

    const real weight = mis_weight_s1_tx(
        scene, camera_subpath, light_subpath, t, select_light);

    return weight * unweighted_contrib;
}

Spectrum weighted_contrib_sx_tx(
    const Scene &scene,
    Vertex *camera_subpath, int s,
    Vertex *light_subpath, int t,
    Sampler &sampler,
    const SceneSampleLightResult &select_light)
{
    const Spectrum unweighted_contrib = unweighted_contrib_sx_tx(
        scene, camera_subpath, s, light_subpath, t, sampler);
    if(!unweighted_contrib.is_finite())
        return Spectrum(REAL_INF);
    if(unweighted_contrib.is_black())
        return {};

    const real weight = mis_weight_sx_tx(
        camera_subpath, s, light_subpath, t, select_light);

    return weight * unweighted_contrib;
}

} // namespace vol_bdpt

AGZ_TRACER_RENDER_END
