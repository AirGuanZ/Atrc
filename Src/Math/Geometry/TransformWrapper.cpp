#include "TransformWrapper.h"

AGZ_NS_BEG(Atrc)

bool TransformWrapper::HasIntersection(const Ray &ray) const
{
    return obj_->HasIntersection(local2World_.ApplyInverseToRay(ray));
}

Option<GeometryIntersection> TransformWrapper::EvalIntersection(const Ray &ray) const
{
    auto tret = obj_->EvalIntersection(local2World_.ApplyInverseToRay(ray));
    if(!tret)
        return std::nullopt;
    return GeometryIntersection{
        tret->t,
        local2World_.ApplyToSurfaceLocal(tret->inct)
    };
}

AGZ_NS_END(Atrc)
