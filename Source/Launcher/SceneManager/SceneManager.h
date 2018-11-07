#pragma once

#include "../Common.h"

// output = {
// 	width  = 640;
// 	height = 480;
// };
// 
// camera = {
// 	eye  = (-5.0, 0.0, 0.0);
// 	dst  = (0.0, 0.0, 0.0);
// 	up   = (0.0, 0.0, 1.0);
// 	FOVy = (Deg, 60);
// };
// 
// entities = (
// 	...
// );
// 
// lights = (
// 
// );

class SceneManager : public AGZ::Singleton<SceneManager>
{
	ObjArena<> arena_;
	Atrc::Scene scene_;

public:

	void Initialize(const ConfigGroup &params);

	bool IsAvailable() const;

	const Atrc::Scene &GetScene() const;
};
