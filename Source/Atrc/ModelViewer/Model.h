#pragma once

#include "Camera.h"
#include "GL.h"
#include "TransformSequence.h"

class Model
{
public:

    struct Vertex
    {
        Vec3f pos;
        Vec3f nor;
        Vec3f tgn;
        Vec2f tex;
    };

    void Initialize(const Vertex *vtxData, uint32_t vtxCount);

    void Render(const Camera &camera) const;

    void DisplayTransformSeq();

private:

    GL::VertexBuffer<Vertex> vtxBuf_;
    GL::VertexArray vao_;

    TransformSequence transSeq_;
};
