#pragma once

#include <limits>
#include <vector>

#include "Camera.h"
#include "Console.h"
#include "Model.h"
#include "ModelDataManager.h"

class ModelManager : public AGZ::Uncopiable
{
public:

    ModelManager();

    ModelManager(ModelManager&&) noexcept = default;

    ModelManager &operator=(ModelManager&&) noexcept = default;

    void Render(const Camera &camera) const;

    void Display(Console &console, const Camera &camera);

    Model *GetSelectedModel() noexcept;

    const Model *GetSelectedModel() const noexcept;

private:

    void NewModelFromData(Console &console, bool clickNew);

    void SortModelByName();

    static constexpr size_t INDEX_NONE = std::numeric_limits<size_t>::max();

    ModelDataManager dataMgr_;

    bool sortByName_;
    size_t selectedIdx_;
    std::vector<Model> models_;
};
