#include <Atrc/Atrc/Camera/Pinhole.h>
#include <Atrc/Atrc/Window.h>

namespace
{
    // returns (hori, vert)
    std::pair<Deg, Deg> Dir2Angle(const Vec3f &dir)
    {
        if(!dir.Length())
            return { Deg(0), Deg(0) };
        Deg vert = Rad(AGZ::Math::Arcsin(dir.Normalize().y));
        Deg hori = dir.x || dir.z ? Rad(AGZ::Math::Arctan2(dir.z, dir.x)) : Rad(0);
        return { hori, vert };
    }

    Vec3f Angle2Dir(Deg hori, Deg vert)
    {
        float cos = Cos(vert);
        return Vec3f(cos * Cos(hori), Sin(vert), cos * Sin(hori));
    }
}


