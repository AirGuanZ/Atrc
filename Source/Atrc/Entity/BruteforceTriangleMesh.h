#pragma once

#include <vector>

#include <Atrc/Common.h>
#include <Atrc/Entity/Entity.h>
#include <Atrc/Math/Math.h>
#include <Atrc/Material/Material.h>

AGZ_NS_BEG(Atrc)

class BruteforceTriangleMesh
    : ATRC_IMPLEMENTS Entity,
      ATRC_PROPERTY AGZ::Uncopiable
{
    struct Triangle
    {
        Vec3r A, B_A, C_A;
        Vec3r nA, nB_nA, nC_nA;
        Vec2r tA, tB_tA, tC_tA;
    };

    std::vector<Triangle> tris_;
    RC<Material> mat_;
    Transform local2World_;

public:

    BruteforceTriangleMesh(
        const AGZ::Model::GeometryMesh &mesh,
        RC<Material> mat, const Transform &local2World);

    bool HasIntersection(const Ray &r) const override;

    bool EvalIntersection(const Ray &r, Intersection *inct) const override;

    AABB GetBoundingBox() const override;

    Real SurfaceArea() const override;

    RC<BxDF> GetBxDF(const Intersection &inct) const override;

};

AGZ_NS_END(Atrc)
