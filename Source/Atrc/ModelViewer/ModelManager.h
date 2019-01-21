#pragma once

#include <limits>
#include <vector>

#include "Camera.h"
#include "Model.h"
#include "ModelDataManager.h"

class ModelManager : public AGZ::Uncopiable
{
public:

    ModelManager();

    ModelManager(ModelManager&&) noexcept = default;

    ModelManager &operator=(ModelManager&&) noexcept = default;

    void Render(const Camera &camera) const;

    void Display();

private:

    static constexpr size_t INDEX_NONE = std::numeric_limits<size_t>::max();

    ModelDataManager dataMgr_;

    size_t selectedIdx_;
    std::vector<Model> models_;
};
