#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterTextureCreators(ResourceManager &rscMgr);

class ConstantTextureCreator : public TextureCreator
{
public:

    ConstantTextureCreator() : TextureCreator("constant") { }

    std::shared_ptr<TextureInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class Constant1TextureCreator : public TextureCreator
{
public:

    Constant1TextureCreator() : TextureCreator("constant1") { }

    std::shared_ptr<TextureInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class ImageTextureCreator : public TextureCreator
{
public:

    ImageTextureCreator() : TextureCreator("image") { }

    std::shared_ptr<TextureInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
