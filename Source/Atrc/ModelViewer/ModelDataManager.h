#pragma once

#include <memory>
#include <vector>

#include <AGZUtils/Utils/Mesh.h>

#include "Console.h"
#include "GL.h"
#include "Model.h"

class ModelDataManager : public AGZ::Uncopiable
{
public:

    using MeshGroup = AGZ::Mesh::GeometryMeshGroup<float>;

    struct MeshGroupData
    {
        AGZ::Str8 name;
        std::string nameText;
        MeshGroup meshGroup;
        AGZ::Mesh::BoundingBox<float> bounding;
        std::shared_ptr<GL::VertexBuffer<Model::Vertex>> vtxBuf;
    };

    void Display(Console &console);

    const MeshGroupData *GetSelectedMeshGroup() const;

private:

    bool Add(const AGZ::Str8 &name, MeshGroup &&meshGroup);

    void LoadFromFile(Console &console, bool newPopup);

    void SortData();

    size_t defaultNameIndex_ = 0;

    bool sortDataByName_ = false;
    int selectedIdx_ = -1;
    std::vector<MeshGroupData> data_;
};
