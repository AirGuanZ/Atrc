#include <limits>

#include "TriangleAux.h"
#include "TriangleMesh.h"

AGZ_NS_BEG(Atrc)

TriangleMesh::TriangleMesh(const Vec3r *ABCs, const Vec2r *uvABCs, size_t triangleCount)
    : verticesCount_(triangleCount * 3)
{
    AGZ_ASSERT(ABCs && uvABCs && triangleCount > 0);

    vertices_ = new Vec3r[verticesCount_];
    uvs_      = new Vec2r[verticesCount_];

    for(size_t i = 0; i < verticesCount_; i += 3)
    {
        vertices_[i] = ABCs[i];
        vertices_[i + 1] = ABCs[i + 1] - ABCs[i];
        vertices_[i + 2] = ABCs[i + 2] - ABCs[i];
        uvs_[i] = uvABCs[i];
        uvs_[i + 1] = uvABCs[i + 1] - uvABCs[i];
        uvs_[i + 2] = uvABCs[i + 2] - uvABCs[i];
    }
}

TriangleMesh::~TriangleMesh()
{
    AGZ_ASSERT(vertices_ && uvs_ && verticesCount_);
    delete[] vertices_;
    delete[] uvs_;
}

bool TriangleMesh::HasIntersection(const Ray &ray) const
{
    AGZ_ASSERT(vertices_ && uvs_ && verticesCount_);

    for(size_t i = 0; i < verticesCount_; i += 3)
    {
        if(TriangleAux::HasIntersection(
            ray, vertices_[i], vertices_[i + 1], vertices_[i + 2]))
            return true;
    }
    return false;
}

Option<GeometryIntersection> TriangleMesh::EvalIntersection(const Ray &ray) const
{
    AGZ_ASSERT(vertices_ && uvs_ && verticesCount_);

    Option<TriangleAux::TriangleIntersection> finalInct = None;
    size_t finalTriangleIndex = 0;
    for(size_t i = 0; i < verticesCount_; i += 3)
    {
        auto inct = TriangleAux::EvalIntersection(
            ray, vertices_[i], vertices_[i + 1], vertices_[i + 2]);
        if(inct && (!finalInct || inct->t < finalInct->t))
        {
            finalTriangleIndex = i;
            finalInct = inct;
        }
    }

    if(finalTriangleIndex == std::numeric_limits<size_t>::max())
        return None;

    auto local = TriangleAux::EvalSurfaceLocal(
        vertices_[finalTriangleIndex + 1], vertices_[finalTriangleIndex + 2],
        uvs_[finalTriangleIndex + 1], uvs_[finalTriangleIndex + 2]);

    Vec2r uv = uvs_[finalTriangleIndex]
             + finalInct->coefB * uvs_[finalTriangleIndex + 1]
             + finalInct->coefC * uvs_[finalTriangleIndex + 2];

    return GeometryIntersection{
        finalInct->t,
        SurfaceLocal(
            ray.At(finalInct->t),
            uv,
            local.normal,
            local.dpdu,
            local.dpdv
        )
    };
}

AGZ_NS_END(Atrc)
