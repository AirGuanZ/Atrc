#include "TransformWrapper.h"

AGZ_NS_BEG(Atrc)

bool TransformWrapper::HasIntersectionImpl(
    const Ray &ray, Real minT, Real maxT
) const
{
    return obj_->HasIntersection(
        Ray(
            local2World_.ApplyInverseToPoint(ray.origin),
            local2World_.ApplyInverseToVector(ray.direction)),
        minT, maxT);
}

Option<Intersection> TransformWrapper::EvalIntersectionImpl(
    const Ray &ray, Real minT, Real maxT
) const
{
    auto tret = obj_->EvalIntersection(
        Ray(
            local2World_.ApplyInverseToPoint(ray.origin),
            local2World_.ApplyInverseToVector(ray.direction)),
        minT, maxT);

    if(!tret)
        return std::nullopt;
    return Intersection{
        tret->t,
        local2World_.ApplyToSurfaceLocal(tret->inct)
    };
}

AGZ_NS_END(Atrc)
