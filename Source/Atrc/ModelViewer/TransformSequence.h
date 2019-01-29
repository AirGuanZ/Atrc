#pragma once

#include <vector>

#include <AGZUtils/Utils/Misc.h>

#include "GL.h"

class TransformSequence : public AGZ::Uncopiable
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

        using Data = std::variant<Translate, RotateX, RotateY, RotateZ, Scale, Matrix>;

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

    int selectedIdx_ = -1;
    std::vector<Transform> transforms_;
    Mat4f mat_;

    void UpdateMat();

    std::optional<Transform> New(const char *title, bool newPopup) const;

    bool EditTransformParam(Transform &trans) const;

public:

    TransformSequence() = default;

    TransformSequence(TransformSequence &&moveFrom) noexcept = default;

    TransformSequence &operator=(TransformSequence &&moveFrom) noexcept = default;

    const Mat4f &GetFinalTransformMatrix() const noexcept;

    void AddMatrixToFront(std::string name, const Mat4f &mat);

    void Display();

    void Clear();
};
