#pragma once

#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

void RegisterTextureCreators(ObjectManager &objMgr);

class ConstantTextureCreator : public TextureCreator
{
public:

    ConstantTextureCreator() : TextureCreator("constant") { }

    std::shared_ptr<TextureInstance> Create(std::string name) const override;
};
