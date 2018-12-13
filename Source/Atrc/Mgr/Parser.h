#pragma once

#include <Atrc/Lib/Core/Transform.h>
#include <Atrc/Mgr/Common.h>

namespace Atrc::Mgr::Parser
{

/*
    (0.1)           => (0.1, 0.1, 0.1)
    (0.1, 0.2, 0.2) => (0.1, 0.2, 0.2)
    b(1)            => (1/255, 1/255, 1/255)
    b(1, 2, 3)      => (1/255, 2/255, 3/255)
*/
Spectrum ParseSpectrum(const ConfigNode &node);

/*
    (1)    => (1, 1)
    (1, 2) => (1, 2, 3)
*/
Vec2i ParseVec2i(const ConfigNode &node);

/*
    (1)    => (1.0, 1.0)
    (1, 2) => (1.0, 2.0, 3.0)
*/
Vec2 ParseVec2(const ConfigNode &node);

/*
    (1)       => (1, 1, 1)
    (1, 2, 3) => (1, 2, 3)
*/
Vec3 ParseVec3(const ConfigNode &node);

/*
    Deg(60)  => Rad(Deg(60))
    Rad(1.4) => Rad(1.4)
*/
Rad ParseAngle(const ConfigNode &node);

/*
    (t0, t1, t2, ..., tn) => t0 * t1 * t2 * ... * tn
    ti = Translate(Real, Real, Real)
       | Rotate(Vec3, Angle)
       | RotateX(Angle)
       | RotateY(Angle)
       | RotateZ(Angle)
       | Scale(Real)
*/
Transform ParseTransform(const ConfigNode &node);

} // namespace Atrc::Mgr::Parser
