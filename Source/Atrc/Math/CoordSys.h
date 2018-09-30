#pragma once

#include <Atrc/Common.h>

AGZ_NS_BEG(Atrc)

class CoordSys
{
public:

    Vec3r ex, ey, ez;

    static CoordSys FromZ(const Vec3r &ez)
    {
        Vec3r ex(AGZ::UNINITIALIZED);
        if(ApproxEq(Dot(ez, Vec3r::UNIT_X()), Real(1), Real(0.1)))
            ex = Cross(ez, Vec3r::UNIT_Y()).Normalize();
        else
            ex = Cross(ez, Vec3r::UNIT_X()).Normalize();
        return { ex, Cross(ez, ex), ez };
    }

    Vec3r C2W(const Vec3r &vec) const
    {
        return ex * vec.x + ey * vec.y + ez * vec.z;
    }
};

AGZ_NS_END(Atrc)
