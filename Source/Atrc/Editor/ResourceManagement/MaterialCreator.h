#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterMaterialCreators(ResourceManager &rscMgr);

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

class NormalizedDiffusionBSSRDFCreator : public MaterialCreator
{
public:

    NormalizedDiffusionBSSRDFCreator() : MaterialCreator("BSSRDF") { }

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
