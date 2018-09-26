#pragma once

#include "../AGZMath.h"
#include "../Geometry.h"
#include "TriangleAux.h"

AGZ_NS_BEG(Atrc)

class TriangleMesh : public GeometryObject
{
    // IMPROVE: memory layout
    Vec3r *vertices_;
    Vec2r *uvs_;

public:

    TriangleMesh(const Vec3r *vertices, const Vec2r *uvs, size_t n);
};

AGZ_NS_END(Atrc)
