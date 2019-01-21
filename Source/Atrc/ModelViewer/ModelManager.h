#pragma once

#include <limits>
#include <vector>

#include "Model.h"
#include "ModelDataManager.h"

class ModelManager : public AGZ::Uncopiable, public AGZ::Unmovable
{
public:

    ModelManager();

private:

    static constexpr size_t INDEX_NONE = std::numeric_limits<size_t>::max();

    ModelDataManager dataMgr_;

    size_t selectedIdx_;
    std::vector<Model> models_;
};
