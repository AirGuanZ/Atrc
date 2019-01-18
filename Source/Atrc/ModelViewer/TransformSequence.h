#pragma once

#include <vector>

#include <AGZUtils/Utils/Misc.h>

#include "GL.h"

class TransformSequence
{
    struct Translate { Vec3f offset; };
    struct RotateX   { Deg angle;    };
    struct RotateY   { Deg angle;    };
    struct RotateZ   { Deg angle;    };
    struct Scale     { float scale;  };

    struct Transform
    {
        AGZ::TypeOpr::Variant<Translate, RotateX, RotateY, RotateZ, Scale> data;

        Mat4f GetMatrix() const noexcept;

        std::string Text() const;
    };

    std::vector<Transform> transforms_;

    AGZ::Option<Transform> New(const char *title, bool newPopup) const;

    void EditTransformParam(Transform &trans) const;

public:

    Mat4f GetFinalTransformMatrix() const noexcept;

    void Display();

    void Clear();
};
