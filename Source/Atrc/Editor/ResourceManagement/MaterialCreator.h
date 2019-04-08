#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterMaterialCreators(ResourceManager &rscMgr);

class BSSRDFInstanceCreator : public MaterialCreator
{
public:

    BSSRDFInstanceCreator() : MaterialCreator("BSSRDF") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneyDiffuseInstanceCreator : public MaterialCreator
{
public:

    DisneyDiffuseInstanceCreator() : MaterialCreator("DisneyDiffuse") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneyReflectionInstanceCreator : public MaterialCreator
{
public:

    DisneyReflectionInstanceCreator() : MaterialCreator("DisneyReflection") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneySpecularInstanceCreator : public MaterialCreator
{
public:

    DisneySpecularInstanceCreator() : MaterialCreator("DisneySpecular") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GGXDielectricInstanceCreator : public MaterialCreator
{
public:

    GGXDielectricInstanceCreator() : MaterialCreator("GGXDielectric") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GGXMetalInstanceCreator : public MaterialCreator
{
public:

    GGXMetalInstanceCreator() : MaterialCreator("GGXMetal") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealBlackInstanceCreator : public MaterialCreator
{
public:

    IdealBlackInstanceCreator() : MaterialCreator("IdealBlack") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealDiffuseInstanceCreator : public MaterialCreator
{
public:

    IdealDiffuseInstanceCreator() : MaterialCreator("IdealDiffuse") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealMirrorInstanceCreator : public MaterialCreator
{
public:

    IdealMirrorInstanceCreator() : MaterialCreator("IdealMirror") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealScalerInstanceCreator : public MaterialCreator
{
public:

    IdealScalerInstanceCreator() : MaterialCreator("IdealScaler") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealSpecularInstanceCreator : public MaterialCreator
{
public:

    IdealSpecularInstanceCreator() : MaterialCreator("IdealSpecular") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class InvisibleSurfaceInstanceCreator : public MaterialCreator
{
public:

    InvisibleSurfaceInstanceCreator() : MaterialCreator("Invisible") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class ONMatteInstanceCreator : public MaterialCreator
{
public:

    ONMatteInstanceCreator() : MaterialCreator("ONMatte") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class TSMetalInstanceCreator : public MaterialCreator
{
public:

    TSMetalInstanceCreator() : MaterialCreator("TSMetal") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
