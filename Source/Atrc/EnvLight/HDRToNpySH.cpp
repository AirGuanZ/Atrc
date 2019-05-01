#include <iostream>

#include <AGZUtils/Utils/Console.h>
#include <AGZUtils/Utils/Texture.h>
#include <Atrc/Core/Core/Light.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Lib/cnpy/cnpy.h>

// impl in NormalToNpySH.cpp
void Light2SH(const Atrc::Light *light, int SHOrder, Atrc::Spectrum *coefs);

void HDRToNpySH(const std::string &imgFilename, int SHOrder, const std::string &outputFilename)
{
    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value" << std::endl;
        return;
    }

    AGZ::Fmt lightConfig(
        "workspace = \"@.\";"
        "light = {{"
        "   type = Environment;"
        "   tex  = {{"
        "       type = HDR;"
        "       filename = \"{}\";"
        "   };"
        "};"
    );
    AGZ::Config config;
    auto configStr = lightConfig.Arg(imgFilename);
    config.LoadFromMemory(configStr);

    Atrc::Mgr::Context ctx(config.Root(), ".");
    RegisterBuiltinCreators(ctx);

    auto light = ctx.Create<Atrc::Light>(config.Root()["light"]);
    int SHC = (SHOrder + 1) * (SHOrder + 1);
    std::vector<Atrc::Spectrum> coefs(SHC);
    Light2SH(light, SHOrder, coefs.data());

    cnpy::npy_save(outputFilename, &coefs[0][0], { size_t(SHC), 3 });
}
