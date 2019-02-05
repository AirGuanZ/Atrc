#pragma once

#include <memory>

#include <Atrc/ModelViewer/Camera.h>
#include <Atrc/ModelViewer/GL.h>
#include <Atrc/ModelViewer/TransformController.h>

struct ModelTransform
{
    Vec3f translate;
    Vec3f rotate;
    float scale = 1;
};

class MeshGroupData;

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

    void Initialize(std::shared_ptr<const MeshGroupData> grpData, const Vec3f &renderColor);

    void Render(const Camera &camera) const;

    void DisplayProperty();

    void DisplayTransform(const Camera &camera);

    const std::string &GetName() const noexcept { return name_; }

private:

    std::string name_;

    std::shared_ptr<const GL::VertexBuffer<Vertex>> vtxBuf_;
    std::shared_ptr<const MeshGroupData> grpData_;
    GL::VertexArray vao_;

    Vec3f renderColor_;

    ModelTransform transform_;
    TransformController transformController_;
};
