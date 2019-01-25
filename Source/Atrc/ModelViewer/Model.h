#pragma once

#include <memory>

#include "Camera.h"
#include "GL.h"
#include "TransformSequence.h"

class Model : public AGZ::Uncopiable
{
public:

    struct Vertex
    {
        Vec3f pos;
        Vec3f nor;
    };

    static void BeginRendering();

    static void EndRendering();

    explicit Model(std::string name) noexcept;

    Model(Model &&moveFrom) noexcept = default;

    Model &operator=(Model &&moveFrom) noexcept = default;

    void Initialize(std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf, const Vec3f &renderColor);

    void Render(const Camera &camera) const;

    void DisplayProperty();

    void DisplayTransformSeq();

    const std::string &GetName() const noexcept { return name_; }

private:

    std::string name_;

    std::shared_ptr<GL::VertexBuffer<Vertex>> vtxBuf_;
    GL::VertexArray vao_;

    Vec3f renderColor_;
    Vec3f displayColor_;
    bool displayColorWith255_;

    TransformSequence transSeq_;
};
