#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterMaterialCreators(ResourceManager &rscMgr);

class BSSRDFCreator : public MaterialCreator
{
public:

    BSSRDFCreator() : MaterialCreator("BSSRDF") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneyDiffuseCreator : public MaterialCreator
{
public:

    DisneyDiffuseCreator() : MaterialCreator("DisneyDiffuse") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneyReflectionCreator : public MaterialCreator
{
public:

    DisneyReflectionCreator() : MaterialCreator("DisneyReflection") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class DisneySpecularCreator : public MaterialCreator
{
public:

    DisneySpecularCreator() : MaterialCreator("DisneySpecular") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GGXDielectricCreator : public MaterialCreator
{
public:

    GGXDielectricCreator() : MaterialCreator("GGXDielectric") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class GGXMetalCreator : public MaterialCreator
{
public:

    GGXMetalCreator() : MaterialCreator("GGXMetal") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealBlackCreator : public MaterialCreator
{
public:

    IdealBlackCreator() : MaterialCreator("IdealBlack") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealDiffuseCreator : public MaterialCreator
{
public:

    IdealDiffuseCreator() : MaterialCreator("IdealDiffuse") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealMirrorCreator : public MaterialCreator
{
public:

    IdealMirrorCreator() : MaterialCreator("IdealMirror") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealScalerCreator : public MaterialCreator
{
public:

    IdealScalerCreator() : MaterialCreator("IdealScaler") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class IdealSpecularCreator : public MaterialCreator
{
public:

    IdealSpecularCreator() : MaterialCreator("IdealSpecular") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class InvisibleSurfaceCreator : public MaterialCreator
{
public:

    InvisibleSurfaceCreator() : MaterialCreator("Invisible") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class ONMatteCreator : public MaterialCreator
{
public:

    ONMatteCreator() : MaterialCreator("ONMatte") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};

class TSMetalCreator : public MaterialCreator
{
public:

    TSMetalCreator() : MaterialCreator("TSMetal") { }

    std::shared_ptr<MaterialInstance> Create(ResourceManager &rscMgr, std::string name) const override;
};
