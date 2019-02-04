#pragma once

#include <memory>
#include <vector>

#include <AGZUtils/Utils/Mesh.h>

#include "Console.h"
#include "FileBrowser.h"
#include "GL.h"
#include "Model.h"

class MeshGroupData
{
    mutable std::shared_ptr<const GL::VertexBuffer<Model::Vertex>> vtxBuf;

public:

    MeshGroupData() = default;
    MeshGroupData(MeshGroupData&&) noexcept = default;
    MeshGroupData &operator=(MeshGroupData&&) noexcept = default;
    MeshGroupData(const MeshGroupData&) = default;
    MeshGroupData &operator=(const MeshGroupData&) = default;

    std::string name;
    AGZ::Mesh::GeometryMeshGroup<float> meshGroup;
    AGZ::Mesh::BoundingBox<float> bounding;
    std::vector<std::string> objNames;

    std::shared_ptr<const GL::VertexBuffer<Model::Vertex>> GetVertexBuffer() const;
};

class ModelDataManager
{
public:

    //using MeshGroup = AGZ::Mesh::GeometryMeshGroup<float>;

    void Display(Console &console);

    std::shared_ptr<const MeshGroupData> GetSelectedMeshGroup() const;

private:

    bool Add(std::string_view name, AGZ::Mesh::GeometryMeshGroup<float> &&meshGroup);

    void DisplayNewData(Console &console, bool newPopup);

    void LoadObj(Console &console, std::string_view filename, std::string_view dataName);

    void SortData();

    size_t defaultNameIndex_ = 0;

    bool sortDataByName_ = false;
    int selectedIdx_ = -1;
    std::vector<std::shared_ptr<MeshGroupData>> data_;

    FileBrowser fileBrowser_;
};
