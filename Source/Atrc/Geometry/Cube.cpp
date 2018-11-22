#include <Atrc/Geometry/Cube.h>

AGZ_NS_BEG(Atrc)

Cube::Cube(const Transform &local2World, Real sideLen)
    : Geometry(local2World), halfSideLen_(Real(0.5) * sideLen)
{
    AGZ_ASSERT(halfSideLen_ > 0);
    Real fac = local2World.ScaleFactor();
    surfaceArea_ = 6 * (fac * fac) * (sideLen * sideLen);
}

bool Cube::HasIntersection(const Ray &r) const
{
    return AABB{ Vec3(-halfSideLen_), Vec3(halfSideLen_) }.HasIntersection(
        local2World_.ApplyInverseToRay(r));
}

namespace
{
    template<int Axis>
    constexpr int NA = (Axis + 1) % 3;
    template<int Axis>
    constexpr int NNA = (Axis + 2) % 3;

    template<int Axis>
    Option<Real> AlignedQuadInct(Real dis2ori, const Ray &r)
    {
        if(!r.dir[Axis])
            return None;
        Real t = (dis2ori - r.ori[Axis]) / r.dir[Axis];

        Vec3 p = r.At(t);
        if(Abs(p[NA<Axis>])  > Abs(dis2ori) ||
           Abs(p[NNA<Axis>]) > Abs(dis2ori))
            return None;

        return r.Between(t) ? Some(t) : None;
    }
}

bool Cube::FindIntersection(const Ray &_r, SurfacePoint *sp) const
{
    Ray r = local2World_.ApplyInverseToRay(_r);

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

    auto nor = Vec3(0.0);
    nor[bestAxis] = negAxis ? -Real(1) : Real(1);

    Vec3 pos = r.At(bestT), ex;

    switch(bestAxis)
    {
    case 0:

        if(negAxis)
        {
            sp->geoUV.u = Clamp(Real(0.5) - Real(0.5) * pos.y / halfSideLen_, Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) - Real(0.5) * pos.z / halfSideLen_, Real(0), Real(1));
            ex = -Vec3::UNIT_Y();
        }
        else
        {
            sp->geoUV.u = Clamp(Real(0.5) * pos.y / halfSideLen_ + Real(0.5), Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) * pos.z / halfSideLen_ + Real(0.5), Real(0), Real(1));
            ex = Vec3::UNIT_Y();
        }

        break;

    case 1:

        if(negAxis)
        {
            sp->geoUV.u = Clamp(Real(0.5) * pos.x / halfSideLen_ + Real(0.5), Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) * pos.z / halfSideLen_ + Real(0.5), Real(0), Real(1));
            ex = Vec3::UNIT_X();
        }
        else
        {
            sp->geoUV.u = Clamp(Real(0.5) - Real(0.5) * pos.x / halfSideLen_, Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) - Real(0.5) * pos.z / halfSideLen_, Real(0), Real(1));
            ex = -Vec3::UNIT_X();
        }

        break;

    case 2:

        if(negAxis)
        {
            sp->geoUV.u = Clamp(Real(0.5) - Real(0.5) * pos.x / halfSideLen_, Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) - Real(0.5) * pos.y / halfSideLen_, Real(0), Real(1));
            ex = -Vec3::UNIT_X();
        }
        else
        {
            sp->geoUV.u = Clamp(Real(0.5) * pos.x / halfSideLen_ + Real(0.5), Real(0), Real(1));
            sp->geoUV.v = Clamp(Real(0.5) * pos.y / halfSideLen_ + Real(0.5), Real(0), Real(1));
            ex = Vec3::UNIT_X();
        }

        break;

    default:
        AGZ::Unreachable();
    }

    sp->t        = bestT;
    sp->pos      = local2World_.ApplyToPoint(pos);
    sp->wo       = -_r.dir;
    sp->usrUV    = sp->geoUV;
    sp->geoLocal = local2World_.ApplyToCoordSystem({ ex, Cross(nor, ex), nor });

    return true;
}

Real Cube::SurfaceArea() const
{
    return surfaceArea_;
}

AABB Cube::LocalBound() const
{
    return { Vec3(-halfSideLen_), Vec3(halfSideLen_) };
}

GeometrySampleResult Cube::Sample() const
{
    int s = AGZ::Math::Random::Uniform(0, 5);
    int axis = s % 3;
    bool neg = s >= 3;

    Vec3 nor(0.0), pos;

    nor[axis] = neg ? -Real(1) : Real(1);

    pos[axis] = neg ? -halfSideLen_ : halfSideLen_;
    pos[(axis + 1) % 3] = AGZ::Math::Random::Uniform(-halfSideLen_, halfSideLen_);
    pos[(axis + 2) % 3] = AGZ::Math::Random::Uniform(-halfSideLen_, halfSideLen_);

    GeometrySampleResult ret;
    ret.pos = local2World_.ApplyToPoint(pos);
    ret.nor = local2World_.ApplyToNormal(nor);
    ret.pdf = 1 / surfaceArea_;

    return ret;
}

Real Cube::SamplePDF([[maybe_unused]] const Vec3 &pos) const
{
    return 1 / surfaceArea_;
}

AGZ_NS_END(Atrc)
