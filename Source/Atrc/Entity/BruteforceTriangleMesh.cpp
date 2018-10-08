#include <Atrc/Entity/BruteforceTriangleMesh.h>

AGZ_NS_BEG(Atrc)

BruteforceTriangleMesh::BruteforceTriangleMesh(
    const AGZ::Model::GeometryMesh &mesh, RC<Material> mat, const Transform &local2World)
    : mat_(mat), local2World_(local2World)
{
    AGZ_ASSERT(mesh.vertices.size() % 3 == 0);
    tris_.reserve(mesh.vertices.size() / 3);
    for(auto i : Between<size_t>(0, mesh.vertices.size(), 3))
    {
        Triangle tri;
        tri.A     = mesh.vertices[i].pos;
        tri.B_A   = mesh.vertices[i + 1].pos - mesh.vertices[i].pos;
        tri.C_A   = mesh.vertices[i + 2].pos - mesh.vertices[i].pos;
        tri.tA    = mesh.vertices[i].tex.xy();
        tri.tB_tA = mesh.vertices[i + 1].tex.xy() - mesh.vertices[i].tex.xy();
        tri.tC_tA = mesh.vertices[i + 2].tex.xy() - mesh.vertices[i].tex.xy();
        tri.nA    = mesh.vertices[i].nor;
        tri.nB_nA = mesh.vertices[i + 1].nor - mesh.vertices[i].nor;
        tri.nC_nA = mesh.vertices[i + 2].nor - mesh.vertices[i].nor;
        tris_.push_back(tri);
    }
}

bool BruteforceTriangleMesh::HasIntersection(const Ray &r) const
{
    Ray _r = local2World_.ApplyInverseToRay(r);
    for(auto &tri : tris_)
    {
        if(Geometry::Triangle::HasIntersection(tri.A, tri.B_A, tri.C_A, _r))
            return true;
    }
    return false;
}

bool BruteforceTriangleMesh::EvalIntersection(const Ray &r, Intersection *inct) const
{
    bool ret = false;
    Ray _r = local2World_.ApplyInverseToRay(r);
    for(auto i : Between<size_t>(0, tris_.size()))
    {
        auto &t = tris_[i];

        Intersection tInct;
        if(Geometry::Triangle::EvalIntersection(t.A, t.B_A, t.C_A, _r, &tInct) &&
           (!ret || tInct.t < inct->t))
        {
            ret = true;
            *inct = tInct;
            inct->flag = i;
            inct->nor = t.nA + inct->uv.u * t.nB_nA + inct->uv.v * t.nC_nA;
        }
    }
    if(ret)
    {
        *inct = local2World_.ApplyToIntersection(*inct);
        inct->entity = this;
    }
    return ret;
}

AABB BruteforceTriangleMesh::GetBoundingBox() const
{
    AABB ret;
    for(auto &t : tris_)
    {
        ret.Expand(local2World_.ApplyToPoint(t.A));
        ret.Expand(local2World_.ApplyToPoint(t.A + t.B_A));
        ret.Expand(local2World_.ApplyToPoint(t.A + t.C_A));
    }
    return ret;
}

Real BruteforceTriangleMesh::SurfaceArea() const
{
    Real ret = 0.0;
    for(auto &t : tris_)
        ret += Geometry::Triangle::SurfaceArea(t.B_A, t.C_A);
    return ret;
}

RC<BxDF> BruteforceTriangleMesh::GetBxDF(const Intersection &inct) const
{
    auto &tri = tris_[inct.flag];
    return mat_->GetBxDF(inct, tri.tA + tri.tB_tA * inct.uv.u + tri.tC_tA * inct.uv.v);
}

AGZ_NS_END(Atrc)
