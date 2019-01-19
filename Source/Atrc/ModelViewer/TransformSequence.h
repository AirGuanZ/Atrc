#pragma once

#include <vector>

#include <AGZUtils/Utils/Misc.h>

#include "GL.h"

class TransformSequence
{
    struct Translate { Vec3f offset;                };
    struct RotateX   { Deg angle;                   };
    struct RotateY   { Deg angle;                   };
    struct RotateZ   { Deg angle;                   };
    struct Scale     { float scale;                 };
    struct Matrix    { std::string text; Mat4f mat; };

    class Transform
    {
    public:

        using Data = AGZ::TypeOpr::Variant<Translate, RotateX, RotateY, RotateZ, Scale, Matrix>;

        explicit Transform(const Data &data);

        const Data &GetData() const noexcept;

        void SetData(const Data &data);

        const Mat4f &GetMatrix() const noexcept;

        const std::string &GetText() const;

    private:

        void UpdateTextAndMat();

        Data data_;
        std::string text_;
        Mat4f mat_;
    };

    std::vector<Transform> transforms_;
    Mat4f mat_;

    void UpdateMat();

    AGZ::Option<Transform> New(const char *title, bool newPopup) const;

    bool EditTransformParam(Transform &trans) const;

public:

    const Mat4f &GetFinalTransformMatrix() const noexcept;

    void AddMatrixToFront(std::string name, const Mat4f &mat);

    void Display();

    void Clear();
};
