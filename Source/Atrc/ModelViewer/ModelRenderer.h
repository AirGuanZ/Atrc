#pragma once

#include "GL.h"

class ModelRenderer
{
public:

    struct Vertex
    {
        Vec3f pos;
        Vec3f nor;
        Vec2f tex;
    };

    void Initialize();

    void SetModelData(const Vertex *vtx, uint32_t vtxCount, const uint32_t *elem, uint32_t elemCount);

    void SetTexture(const GL::Texture2D *tex) noexcept { tex_ = tex; }

    const Mat4f &GetWorld() const noexcept { return world_; }
    const Mat4f &GetView()  const noexcept { return view_;  }
    const Mat4f &GetProj()  const noexcept { return proj_;  }

    void SetWorld(const Mat4f &world);
    void SetView(const Mat4f &view);
    void SetProj(const Mat4f &proj);

    void Render() const;

private:

    bool hasData_ = false;

    GL::VertexBuffer<Vertex> vtxBuf_;
    GL::ElementBuffer<uint32_t> elemBuf_;
    
    GL::Program program_;
    const GL::Texture2D *tex_ = nullptr;
    GL::UniformVariableAssignment uniforms_;

    GL::VertexArray vao_;

    Mat4f world_;
    Mat4f view_;
    Mat4f proj_;
    Mat4f WVP_;
};
