#include <Atrc/Core/Core/Light.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>
#include <Atrc/Mgr/Context.h>
#include <Atrc/SH2D/Light2SH.h>
#include <Atrc/SH2D/ProjectLight.h>
#include <Lib/cnpy/cnpy.h>

void ProjectLight(const AGZ::Config &config, const std::string &configPath, const ProjectLightOutputArgs &outputArgs)
{
    using namespace Atrc;

    auto &root = config.Root();
    Mgr::Context ctx(root, configPath);
    Mgr::RegisterBuiltinCreators(ctx);

    auto light = ctx.Create<Light>(root["light"]);
    int N = root["N"].Parse<int>();

    int SHOrder = root["SHOrder"].Parse<int>();
    int SHC = (SHOrder + 1) * (SHOrder + 1);

    if(SHOrder < 0 || SHOrder > 4)
    {
        std::cout << "Invalid SHOrder value: " << SHOrder << std::endl;
        return;
    }

    std::vector<Spectrum> coefs(SHC);
    Light2SH(light, SHOrder, N, coefs.data());

    if(outputArgs.outputTXT)
    {
        auto filename = outputArgs.dir / "light.txt";

        std::ofstream fout(filename);
        if(!fout)
        {
            std::cout << "Failed to open output file " << filename.string() << std::endl;
            return;
        }

        fout << SHOrder << " ";
        for(int i = 0; i < SHC; ++i)
        {
            fout << coefs[i].r << " ";
            fout << coefs[i].g << " ";
            fout << coefs[i].b << " ";
        }

        if(!fout)
            std::cout << "Failed to save light coefs to " << filename.string() << std::endl;
    }

    if(outputArgs.outputNPZ)
    {
        auto filename = outputArgs.dir / "light.npz";
        cnpy::npz_save(filename.string(), "light", &coefs[0][0], { size_t(SHC), 3 });
    }
}
