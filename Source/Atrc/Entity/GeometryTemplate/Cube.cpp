#include <Atrc/Entity/GeometryTemplate/Cube.h>

AGZ_NS_BEG(Atrc)

Cube::Cube(Real halfSideLen)
    : halfSideLen_(halfSideLen)
{

}

bool Cube::HasIntersection(const Ray &r) const
{
    return AABB{ Vec3r(-halfSideLen_), Vec3r(halfSideLen_) }.HasIntersection(r);
}

namespace
{
    template<int Axis>
    Option<Real> AlignedQuadInct(Real dis2ori, const Ray &r)
    {
        if(!r.direction[Axis])
            return None;
        Real t = (dis2ori - r.origin[Axis]) / r.direction[Axis];

        Vec3r p = r.At(t);
        if(Abs(p[(Axis + 1) % 3]) > Abs(dis2ori) ||
           Abs(p[(Axis + 2) % 3]) > Abs(dis2ori))
            return None;

        return t < r.minT || t > r.maxT ? None : Some(t);
    }
}

bool Cube::EvalIntersection(const Ray &r, Intersection *inct) const
{
    int bestAxis = -1;
    bool negAxis = false;
    Real bestT = 0.0;

    auto posX = AlignedQuadInct<0>(halfSideLen_, r);
    if(posX)
    {
        bestAxis = 0;
        bestT = *posX;
    }

    auto negX = AlignedQuadInct<0>(-halfSideLen_, r);
    if(negX && (bestAxis < 0 || *negX < bestT))
    {
        bestAxis = 0;
        negAxis = true;
        bestT = *negX;
    }

    auto posY = AlignedQuadInct<1>(halfSideLen_, r);
    if(posY && (bestAxis < 0 || *posY < bestT))
    {
        bestAxis = 1;
        negAxis = false;
        bestT = *posY;
    }

    auto negY = AlignedQuadInct<1>(-halfSideLen_, r);
    if(negY && (bestAxis < 0 || *negY < bestT))
    {
        bestAxis = 1;
        negAxis = true;
        bestT = *negY;
    }

    auto posZ = AlignedQuadInct<2>(halfSideLen_, r);
    if(posZ && (bestAxis < 0 || *posZ < bestT))
    {
        bestAxis = 2;
        negAxis = false;
        bestT = *posZ;
    }

    auto negZ = AlignedQuadInct<2>(-halfSideLen_, r);
    if(negZ && (bestAxis < 0 || *negZ < bestT))
    {
        bestAxis = 2;
        negAxis = true;
        bestT = *negZ;
    }

    if(bestAxis < 0)
        return false;

    inct->pos = r.At(bestT);
    inct->wr = -r.direction;

    auto nor = Vec3r(0.0);
    nor[bestAxis] = negAxis ? -1.0 : 1.0;
    inct->nor = nor;

    inct->t = bestT;

    // TODO
    inct->uv = Vec2r(0.0);

    inct->entity = this;

    return true;
}

AABB Cube::GetBoundingBox() const
{
    return AABB{ Vec3r(-halfSideLen_), Vec3r(halfSideLen_) };
}

Real Cube::SurfaceArea() const
{
    Real ret = halfSideLen_ + halfSideLen_;
    ret *= ret;
    return 6.0 * ret;
}

AGZ_NS_END(Atrc)
