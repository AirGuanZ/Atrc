#pragma once

#include "../../Common.h"
#include "../AGZMath.h"
#include "../Geometry.h"

AGZ_NS_BEG(Atrc)

class TriangleMesh : ATRC_IMPLEMENTS GeometryObject, ATRC_PROPERTY AGZ::Uncopiable
{
    // IMPROVE: layout

    // {A,  B_A,  C_A}   * triangleCount
    Vec3r *vertices_;
    // {uvA,uvB_A,uvC_A} * triangleCount
    // IMPROVE: saves dpdu and dpdv
    Vec2r *uvs_;

    size_t verticesCount_;

public:

    TriangleMesh(const Vec3r *ABCs, const Vec2r *uvABCs, size_t triangleCount);

    ~TriangleMesh();

    bool HasIntersection(const Ray &ray) const override;

    Option<GeometryIntersection> EvalIntersection(const Ray &ray) const override;
};

AGZ_NS_END(Atrc)
