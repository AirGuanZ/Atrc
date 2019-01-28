#pragma once

#include <memory>
#include <vector>

#include <AGZUtils/Utils/Mesh.h>

#include "Console.h"
#include "FileBrowser.h"
#include "GL.h"
#include "Model.h"

class ModelDataManager
{
public:

    using MeshGroup = AGZ::Mesh::GeometryMeshGroup<float>;

    class MeshGroupData
    {
		mutable std::shared_ptr<const GL::VertexBuffer<Model::Vertex>> vtxBuf;

    public:

		MeshGroupData()                                    = default;
		MeshGroupData(MeshGroupData&&) noexcept            = default;
		MeshGroupData &operator=(MeshGroupData&&) noexcept = default;
        MeshGroupData(const MeshGroupData&)                = default;
        MeshGroupData &operator=(const MeshGroupData&)     = default;

        std::string name;
        MeshGroup meshGroup;
        AGZ::Mesh::BoundingBox<float> bounding;

		std::shared_ptr<const GL::VertexBuffer<Model::Vertex>> GetVertexBuffer() const;
    };

    void Display(Console &console);

    const MeshGroupData *GetSelectedMeshGroup() const;

private:

    bool Add(const std::string &name, MeshGroup &&meshGroup);

    void LoadFromFile(Console &console, bool newPopup);

    void SortData();

    size_t defaultNameIndex_ = 0;

    bool sortDataByName_ = false;
    int selectedIdx_ = -1;
    std::vector<MeshGroupData> data_;

    FileBrowser fileBrowser_;
};
