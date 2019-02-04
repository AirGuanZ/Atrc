#pragma once

#include <limits>
#include <vector>

#include <AGZUtils/Misc/Singleton.h>

#include "Material.h"

class MaterialCreatorTable : public AGZ::Singleton<MaterialCreatorTable>
{
public:

    MaterialCreatorTable();

    const MaterialCreator &operator[](size_t idx) const;

    size_t GetTableSize() const { return creators_.size(); }

private:

    std::vector<const MaterialCreator*> creators_;
};

class MaterialCreatorSelector
{
public:

    MaterialCreatorSelector();

    void Display(const char *label);

    const MaterialCreator *GetSelectedMaterialCreator() const;

private:

    const MaterialCreator *selectedMaterialCreator_;
};

class MaterialPool
{
public:

    MaterialPool();

    void Display();

    std::shared_ptr<const MaterialInstance> GetSelectedMaterial() const;

private:

    static constexpr size_t INDEX_NONE = (std::numeric_limits<size_t>::max)();

    size_t selectedMaterialIndex_;
    std::vector<std::shared_ptr<MaterialInstance>> materials_;
};
