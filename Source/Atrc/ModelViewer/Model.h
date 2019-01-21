#pragma once

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

    void Initialize(const Vertex *vtxData, uint32_t vtxCount, const Vec3f &renderColor);

    void Render(const Camera &camera) const;

    void DisplayProperty();

    void DisplayTransformSeq();

private:

    std::string name_;

    GL::VertexBuffer<Vertex> vtxBuf_;
    GL::VertexArray vao_;

    Vec3f renderColor_;
    Vec3f displayColor_;
    bool displayColorWith255_;

    TransformSequence transSeq_;
};
