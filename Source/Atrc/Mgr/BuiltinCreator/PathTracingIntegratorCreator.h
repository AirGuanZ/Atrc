#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinPathTracingIntegratorCreators(Context &context);

/*
    type = Full

    minDepth       = int
    maxDepth       = int
    contProb       = Real
    sampleAllLight = True | False
*/
class FullPathTracingIntegratorCreator : public Creator<PathTracingIntegrator>
{
public:

    Str8 GetTypeName() const override { return "Full"; }

    PathTracingIntegrator *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = MIS

    minDepth       = int
    maxDepth       = int
    contProb       = Real
    sampleAllLight = True | False
*/
class MISPathTracingIntegratorCreator : public Creator<PathTracingIntegrator>
{
public:

    Str8 GetTypeName() const override { return "MIS"; }

    PathTracingIntegrator *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Native

    minDepth = int
    maxDepth = int
    contProb = Real
*/
class NativePathTracingIntegratorCreator : public Creator<PathTracingIntegrator>
{
public:

    Str8 GetTypeName() const override { return "Native"; }

    PathTracingIntegrator *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = ShadingNormal
*/
class ShadingNormalIntegratorCreator: public Creator<PathTracingIntegrator>
{
public:

    Str8 GetTypeName() const override { return "ShadingNormal"; }

    PathTracingIntegrator *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
