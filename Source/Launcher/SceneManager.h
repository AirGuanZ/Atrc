#pragma once

#include <ObjMgr/ObjectManager.h>

// output = {
//     width  = 640;
//     height = 480;
// };
// 
// camera = {
//     eye  = (-5.0, 0.0, 0.0);
//     dst  = (0.0, 0.0, 0.0);
//     up   = (0.0, 0.0, 1.0);
//     FOVy = (Deg, 60);
// };
// 
// entities = (
//     ...
// );
// 
// lights = (
// 
// );

class SceneManager : public AGZ::Singleton<SceneManager>
{
    AGZ::ObjArena<> arena_;
    Atrc::Scene scene_;

public:

    void Initialize(const AGZ::ConfigGroup &params);

    bool IsAvailable() const;

    const Atrc::Scene &GetScene() const;
};
