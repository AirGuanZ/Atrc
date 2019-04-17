#include <Atrc/Editor/Light/CubeEnv.h>

namespace Atrc::Editor
{

std::string CubeEnv::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "posX = {};"
        "posY = {};"
        "posZ = {}:"
        "negX = {};"
        "negY = {};"
        "negZ = {};"
        "rotateYDeg = {};"
    );

    return Wrap(fmt.Arg(
        GetType(),
        posX_.GetNoneNullResource()->Save(relPath),
        posY_.GetNoneNullResource()->Save(relPath),
        posZ_.GetNoneNullResource()->Save(relPath),
        negX_.GetNoneNullResource()->Save(relPath),
        negY_.GetNoneNullResource()->Save(relPath),
        negZ_.GetNoneNullResource()->Save(relPath),
        rotateYDeg_
    ));

    AGZ_HIERARCHY_WRAP("in saving cube environment light")
}

void CubeEnv::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto posX = RF.CreateAndLoad<ITexture>(params["posX"], relPath);
    auto posY = RF.CreateAndLoad<ITexture>(params["posY"], relPath);
    auto posZ = RF.CreateAndLoad<ITexture>(params["posZ"], relPath);
    auto negX = RF.CreateAndLoad<ITexture>(params["negX"], relPath);
    auto negY = RF.CreateAndLoad<ITexture>(params["negY"], relPath);
    auto negZ = RF.CreateAndLoad<ITexture>(params["negZ"], relPath);

    posX_.SetResource(posX);
    posY_.SetResource(posY);
    posZ_.SetResource(posZ);
    negX_.SetResource(negX);
    negY_.SetResource(negY);
    negZ_.SetResource(negZ);

    rotateYDeg_ = params["rotateYDeg"].Parse<float>();

    AGZ_HIERARCHY_WRAP("in loading cube environment light with " + params.ToString())
}

std::string CubeEnv::Export(const std::filesystem::path &path) const
{
    AGZ_HIERARCHY_TRY

    static const AGZ::Fmt fmt(
        "type = {};"
        "posX = {};"
        "posY = {};"
        "posZ = {}:"
        "negX = {};"
        "negY = {};"
        "negZ = {};"
        "transform = (RotateY(Deg({})));"
        "rotateYDeg = {};"
    );

    return Wrap(fmt.Arg(
        GetType(),
        posX_.GetNoneNullResource()->Save(path),
        posY_.GetNoneNullResource()->Save(path),
        posZ_.GetNoneNullResource()->Save(path),
        negX_.GetNoneNullResource()->Save(path),
        negY_.GetNoneNullResource()->Save(path),
        negZ_.GetNoneNullResource()->Save(path),
        rotateYDeg_,
        rotateYDeg_
    ));

    AGZ_HIERARCHY_WRAP("in exporting cube environment light")
}

void CubeEnv::Display()
{
    posX_.DisplayAsSubresource("posX");
    posY_.DisplayAsSubresource("posY");
    posZ_.DisplayAsSubresource("posZ");
    negX_.DisplayAsSubresource("negX");
    negY_.DisplayAsSubresource("negY");
    negZ_.DisplayAsSubresource("negZ");
    
    float rad = AGZ::Math::Deg2Rad(Deg(rotateYDeg_)).value;
    ImGui::SliderAngle("rotateY", &rad, 0.0f, 360.0f);
    rotateYDeg_ = AGZ::Math::Rad2Deg(Rad(rad)).value;
}

bool CubeEnv::IsMultiline() const noexcept
{
    return true;
}

} // namespace Atrc::Editor

