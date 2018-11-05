#include <Atrc/Integrator/VolumetricPathTracer.h>

AGZ_NS_BEG(Atrc)

VolumetricPathTracer::VolumetricPathTracer(int maxDepth)
	: maxDepth_(maxDepth)
{
	AGZ_ASSERT(maxDepth >= 1);
}

Spectrum VolumetricPathTracer::GetRadiance(const Scene &scene, const Ray &r, AGZ::ObjArena<> &arena) const
{
	return L_left(scene, r, 1, arena);
}

Spectrum VolumetricPathTracer::L_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const
{
	AGZ_ASSERT(sp.entity);
	
	auto light = sp.entity->AsLight();
	Spectrum le = light ? light->AreaLe(sp) : SPECTRUM::BLACK;

	return le + L2_right(scene, sp, depth, arena);
}

Spectrum VolumetricPathTracer::L2_right(const Scene &scene, const SurfacePoint &sp, int depth, AGZ::ObjArena<> &arena) const
{
	ShadingPoint shdPnt;
	sp.entity->GetMaterial(sp)->Shade(sp, &shdPnt, arena);

	auto e1 = E1_right(scene, sp, shdPnt, depth, arena);
	auto e2 = E2_right(scene, sp, shdPnt, depth, arena);
	auto s  = S_right(scene, sp, shdPnt, depth, arena);

	return e1 + e2 + s;
}

Spectrum VolumetricPathTracer::E1_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const
{
	auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
	if(!bsdfSample || !bsdfSample->coef)
		return Spectrum();

	SurfacePoint newInct;
	Ray newRay(sp.pos, bsdfSample->wi, EPS);
	newRay.medium = sp.mediumInterface.GetMedium(sp.geoLocal.ez, bsdfSample->wi);

	if(scene.FindCloestIntersection(newRay, &newInct))
	{
		auto light = newInct.entity->AsLight();
		if(!light)
			return Spectrum();

		Spectrum tr = newRay.medium ? newRay.medium->Tr(sp.pos, newInct.pos) : Spectrum(1.0f);

		if(bsdfSample->type & BXDF_SPECULAR)
		{
			return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
				 * tr * light->AreaLe(newInct) / bsdfSample->pdf;
		}

		Real lpdf = scene.SampleLightPDF(light) * light->SampleLiPDF(newInct.pos, sp);
		return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
			 * tr * light->AreaLe(newInct) / (bsdfSample->pdf + lpdf);
	}

	auto [light, lpdf] = scene.SampleLight();

	if(!light || light->IsDelta())
		return Spectrum();

	// NonareaLight可以选择忽略全局介质，否则直接挂掉
	if(!light->IgnoreFirstMedium() && newRay.medium)
		return Spectrum();

	auto le = light->NonareaLe(newRay);
	if(!le)
		return Spectrum();

	if(bsdfSample->type & BXDF_SPECULAR)
	{
		return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
             * le / bsdfSample->pdf;
	}

	lpdf *= light->SampleLiPDF(sp.pos + bsdfSample->wi, sp, false);
	return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * le / (bsdfSample->pdf + lpdf);
}

Spectrum VolumetricPathTracer::E2_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const
{
	auto [light, lpdf] = scene.SampleLight();
	if(!light)
		return Spectrum();

	auto lightSample = light->SampleLi(sp);
	if(!lightSample.radiance)
		return Spectrum();

	Spectrum f = shd.bsdf->Eval(lightSample.wi, sp.wo, BXDF_ALL);
	if(!f)
		return Spectrum();

	Ray shadowRay(sp.pos, lightSample.wi, EPS, (lightSample.pos - sp.pos).Length() - EPS);
	if(scene.HasIntersection(shadowRay) ||
		((Dot(sp.wo,		  sp.geoLocal.ez) <= 0.0) ^
		 (Dot(lightSample.wi, sp.geoLocal.ez) <= 0.0)))
		return Spectrum();

	Spectrum tr;
	if(light->IgnoreFirstMedium())
		tr = Spectrum(1.0f);
	else
	{
		auto medium = sp.mediumInterface.GetMedium(sp.geoLocal.ez, lightSample.wi);
		tr = medium ? medium->Tr(lightSample.pos, sp.pos) : Spectrum(1.0f);
	}

	lpdf *= lightSample.pdf;

	if(light->IsDelta())
	{
		return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
		     * tr * lightSample.radiance / lpdf;
	}

	Real bpdf = shd.bsdf->SampleWiPDF(lightSample.wi, sp.wo, BxDFType(BXDF_ALL & ~BXDF_SPECULAR));
	return f * Abs(Dot(sp.geoLocal.ez, lightSample.wi))
		 * tr * lightSample.radiance / (bpdf + lpdf);
}

Spectrum VolumetricPathTracer::S_right(const Scene &scene, const SurfacePoint &sp, const ShadingPoint &shd, int depth, AGZ::ObjArena<> &arena) const
{
	auto bsdfSample = shd.bsdf->SampleWi(sp.wo, BXDF_ALL);
	if(!bsdfSample || !bsdfSample->coef)
		return Spectrum();

	Ray newRay(sp.pos, bsdfSample->wi, EPS);
	auto d2 = D2_left(scene, newRay, depth + 1, arena);

	if(!d2)
		return Spectrum();

	return bsdfSample->coef * Abs(Dot(shd.shdLocal.ez, bsdfSample->wi))
         * d2 / bsdfSample->pdf;
}

Spectrum VolumetricPathTracer::Ls_right(const Scene &scene, const MediumPoint &mp, int depth, AGZ::ObjArena<> &arena) const
{
	MediumShadingPoint medShd;
	mp.medium->Shade(mp, &medShd, arena);

	// 对相函数采样

	auto phSample = medShd.ph->SampleWi();
	if(!phSample.coef)
		return medShd.le;

	Ray phRay(mp.pos, phSample.wi, EPS);
	auto lph = L_left(scene, phRay, depth + 1, arena);

	return medShd.le + medShd.sigmaS * phSample.coef * lph;
}

// 若r发射出去有交点，则按https://airguanz.github.io/2018/10/28/LTE-with-participating-medium.html
// 中的公式来；否则采样自发光，再加上non-area光源的发光
Spectrum VolumetricPathTracer::D2_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const
{
	// TODO
	return Spectrum(maxDepth_);
}

Spectrum VolumetricPathTracer::L_left(const Scene &scene, const Ray &r, int depth, AGZ::ObjArena<> &arena) const
{
	// TODO
	return Spectrum(maxDepth_);
}

AGZ_NS_END(Atrc)
