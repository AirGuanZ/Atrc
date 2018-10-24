#include <queue>

#include <Atrc/Geometry/TriangleBVH.h>

AGZ_NS_BEG(Atrc)

namespace
{
    Real TriangleSurfaceArea(const Vec3 &B_A, const Vec3 &C_A)
    {
        return 0.5 * Cross(B_A, C_A).Length();
    }

    bool HasIntersectionWithTriangle(const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A, const Ray &r)
    {
        Vec3 s1 = Cross(r.dir, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.ori - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0 || alpha > 1)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.dir, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;
        return r.Between(t);
    }

    // ½öÉèÖÃspµÄt£¬pos£¬wo£¬geoUV
    bool EvalIntersectionWithTriangle(
        const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A,
        const Ray &r, SurfacePoint *sp)
    {
        AGZ_ASSERT(inct);

        Vec3 s1 = Cross(r.dir, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.ori - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0 || alpha > 1)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.dir, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;
        if(t < r.minT || t > r.maxT)
            return false;

        sp->t     = t;
        sp->pos   = r.At(t);
        sp->wo    = -r.dir.Normalize();
        sp->geoUV = Vec2(alpha, beta);

        return true;
    }
}

TriangleBVHCore::TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount)
    : areaPrefixSum_(triangleCount)
{
    InitBVH(vertices, triangleCount);

    AGZ_ASSERT(!triangles_.empty());

    areaPrefixSum_[0] = TriangleSurfaceArea(triangles_[0].B_A, triangles_[0].C_A);
    for(uint32_t i = 1; i < triangles_.size(); ++i)
    {
        areaPrefixSum_[i] = areaPrefixSum_[i - 1] +
            TriangleSurfaceArea(triangles_[i].B_A, triangles_[i].C_A);
    }
}

bool TriangleBVHCore::HasIntersection(const Ray &r) const
{
    AGZ_ASSERT(!nodes_.empty());

    std::queue<uint32_t> tasks;
    tasks.push(0);
    while(!tasks.empty())
    {
        uint32_t taskNodeIdx = tasks.front();
        tasks.pop();
        const Node &node = nodes_[taskNodeIdx];

        if(node.isLeaf)
        {
            for(uint32_t i = node.leaf.start; i < node.leaf.end; ++i)
            {
                const auto &tri = triangles_[i];
                if(HasIntersectionWithTriangle(tri.A, tri.B_A, tri.C_A, r))
                    return true;
            }
        }
        else if(node.internal.bound->HasIntersection(r))
        {
            tasks.push(taskNodeIdx + 1);
            tasks.push(node.internal.rightChild);
        }
    }

    return false;
}

bool TriangleBVHCore::EvalIntersection(const Ray &r, SurfacePoint *sp) const
{
    AGZ_ASSERT(!nodes_.empty());

    bool ret = false;
    std::queue<uint32_t> tasks;
    tasks.push(0);
    while(!tasks.empty())
    {
        uint32_t taskNodeIdx = tasks.front();
        tasks.pop();
        const Node &node = nodes_[taskNodeIdx];

        if(node.isLeaf)
        {
            SurfacePoint tSp;
            for(uint32_t i = node.leaf.start; i < node.leaf.end; ++i)
            {
                const auto &tri = triangles_[i];
                if(EvalIntersectionWithTriangle(
                    tri.A, tri.B_A, tri.C_A, r, &tSp) && (!ret || tSp.t < sp->t))
                {
                    ret = true;
                    *sp = tSp;
                    sp->flag0 = i;
                }
            }
        }
        else if(node.internal.bound->HasIntersection(r))
        {
            tasks.push(taskNodeIdx + 1);
            tasks.push(node.internal.rightChild);
        }
    }

    if(!ret)
        return false;

    const auto &tri = triangles_[sp->flag0];
    sp->usrUV = tri.tA + sp->geoUV.u * tri.tB_tA + sp->geoUV.v * tri.tC_tA;
    sp->geoLocal = LocalCoordSystem::FromEz(
        tri.nA + sp->geoUV.u * tri.nB_nA + sp->geoUV.v * tri.nC_nA);
    return true;
}

AABB TriangleBVHCore::LocalBound() const
{
    AGZ_ASSERT(!nodes_.empty());

    if(nodes_[0].isLeaf)
    {
        AABB ret;
        for(auto &tri : triangles_)
        {
            ret.Expand(tri.A)
               .Expand(tri.A + tri.B_A)
               .Expand(tri.A + tri.C_A);
        }
        return ret;
    }

    return *nodes_[0].internal.bound;
}

Real TriangleBVHCore::SurfaceArea() const
{
    AGZ_ASSERT(!areaPrefixSum_.empty());
    return areaPrefixSum_.back();
}

AGZ_NS_END(Atrc)
