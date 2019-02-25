#pragma once

#include <Atrc/Mgr/Context.h>

namespace Atrc::Mgr
{

void RegisterBuiltinMaterialCreators(Context &context);

/*
    type = BSSRDF

    surface = Material
    A       = Texture
    mfp     = Texture
    eta     = Real
*/
class BSSRDFSurfaceCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "BSSRDF"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = DisneyBRDF;

    baseColor      = Texture
    subsurface     = Texture
    metallic       = Texture
    specular       = Texture
    specularTint   = Texture
    roughness      = Texture
    anisotropic    = Texture
    sheen          = Texture
    sheenTint      = Texture
    clearcoat      = Texture
    clearcoatGloss = Texture

    normalMapper = Texture | null
 */
class DisneyBRDFMaterialCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "DisneyBRDF"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = DisneyDiffuse;

    baseColor      = Texture
    subsurface     = Texture
    
    roughness      = Texture

    normalMapper = Texture | null
 */
class DisneyDiffuseCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "DisneyDiffuse"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = DisneySpecular;

    baseColor    = Texture
    specular     = Texture
    specularTint = Texture
    metallic     = Texture
    roughness    = Texture
    anisotropic  = Texture

    normalMapper = Texture | null
 */
class DisneySpecularCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "DisneySpecular"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = GGXDielectric

    fresnel   = Dielectric
    rc        = Texture
    roughness = Texture

    normalMapper = Texture | null
*/
class GGXDielectricCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "GGXDielectric"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = GGXMetal

    fresnel   = Fresnel
    rc        = Texture
    roughness = Texture

    normalMapper = Texture | null
*/
class GGXMetalCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "GGXMetal"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealBlack
*/
class IdealBlackCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "IdealBlack"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealDiffuse

    albedo = Texture

    normalMapper = Texture | null
*/
class IdealDiffuseCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "IdealDiffuse"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealMirror

    rc = Texture
    fresnel = Fresnel
*/
class IdealMirrorCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "IdealMirror"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealScaler

    scale = Texture
    internal = Material
*/
class IdealScalerCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "IdealScaler"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = IdealSpecular

    rc = Texture
    fresnel = Dielectric
*/
class IdealSpecularCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "IdealSpecular"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = Invisible
*/
class InvisibleSurfaceCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "Invisible"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = ONMatte

    albedo = Texture
    sigma  = Texture

    normalMapper = Texture | null
*/
class ONMatteCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "ONMatte"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

/*
    type = TSMetal

    fresnel   = Fresnel
    rc        = Texture
    roughness = Texture

    normalMapper = Texture | null
*/
class TSMetalCreator : public Creator<Material>
{
public:

    std::string GetTypeName() const override { return "TSMetal"; }

    Material *Create(const ConfigGroup &group, Context &context, Arena &arena) const override;
};

} // namespace Atrc::Mgr
