#pragma once

#include <Atrc/Editor/ResourceManagement/ResourceManager.h>

void RegisterMaterialCreators(ResourceManager &rscMgr);

class IdealBlackCreator : public MaterialCreator
{
public:

    IdealBlackCreator() : MaterialCreator("black") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealDiffuseCreator : public MaterialCreator
{
public:

    IdealDiffuseCreator() : MaterialCreator("diffuse") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealMirrorCreator : public MaterialCreator
{
public:

    IdealMirrorCreator() : MaterialCreator("mirror") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealScalerCreator : public MaterialCreator
{
public:

    IdealScalerCreator() : MaterialCreator("scaler") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class IdealSpecularCreator : public MaterialCreator
{
public:

    IdealSpecularCreator() : MaterialCreator("specular") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class InvisibleSurfaceCreator : public MaterialCreator
{
public:

    InvisibleSurfaceCreator() : MaterialCreator("invisible") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class NormalizedDiffusionBSSRDFCreator : public MaterialCreator
{
public:

    NormalizedDiffusionBSSRDFCreator() : MaterialCreator("nd-bssrdf") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class ONMatteCreator : public MaterialCreator
{
public:

    ONMatteCreator() : MaterialCreator("on-matte") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};

class TSMetalCreator : public MaterialCreator
{
public:

    TSMetalCreator() : MaterialCreator("ts-metal") { }

    std::shared_ptr<MaterialInstance> Create(std::string name) const override;
};
