#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinPathTracingIntegratorCreators(Context &context);

/*
    type = MISPathTracing

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
    type = NativePathTracing

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

/*
    type = Vol

    minDepth       = int
    maxDepth       = int
    contProb       = Real
    sampleAllLight = True | False
    lightInMedium  = True | False | null(True)
*/
class VolPathTracingIntegratorCreator : public Creator<PathTracingIntegrator>
{
public:

    Str8 GetTypeName() const override { return "Vol"; }

    PathTracingIntegrator *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
