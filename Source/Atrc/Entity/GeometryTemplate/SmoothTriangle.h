#pragma once

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>

AGZ_NS_BEG(Atrc)

class SmoothTriangle
    : ATRC_IMPLEMENTS Entity
{
    Vec3r A, B_A, C_A;
    Vec3r nA, nB_nA, nC_nA;

public:

    SmoothTriangle(const Vec3r (&vertices)[3], const Vec3r (&normals)[3]);
    
    bool HasIntersection(const Ray &r) const override;
    
    bool EvalIntersection(const Ray &r, Intersection *inct) const override;
    
    AABB GetBoundingBox() const override;
};

AGZ_NS_END(Atrc)
