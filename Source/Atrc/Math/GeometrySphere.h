#pragma once

#include <Atrc/Common.h>
#include <Atrc/Math/CoordSys.h>
#include <Atrc/Math/Ray.h>

AGZ_NS_BEG(Atrc::Geometry::Sphere)

bool HasIntersection(const Ray &r, Real radius);

bool EvalIntersection(const Ray &r, Real radius, Intersection *inct);

Vec2r EvalUV(const Ray &r, Real radius, const Intersection &inct);

CoordSys EvalTangentSpace(const Ray &r, Real radius, const Intersection &inct);
    
AGZ_NS_END(Atrc::Geometry::Sphere)
