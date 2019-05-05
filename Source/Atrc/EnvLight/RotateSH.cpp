#include <filesystem>
#include <string>

#include <AGZUtils/Utils/Math.h>
#include <Atrc/EnvLight/Internal.h>
#include <Lib/cnpy/cnpy.h>

void RotateSH(const std::string &inFilename, const std::string &outFilename, float xDeg, float yDeg, float zDeg)
{
    auto [SHOrder, coefs] = LoadSHFromNPY(inFilename);
    if(!SHOrder)
    {
        std::cout << "Failed to load light sh coefs from " << inFilename << std::endl;
        return;
    }
    std::cout << "SHOrder: " << SHOrder << std::endl;
    int SHC = SHOrder * SHOrder;

    std::vector<float> channels[3];
    for(int c = 0; c < 3; ++c)
    {
        channels[c].resize(SHC);
        for(int i = 0; i < SHC; ++i)
            channels[c][i] = coefs[i][c];
    }

    using AGZ::Math::Degf;
    auto rotateMat = AGZ::Math::RM_Mat3f::RotateX(Degf(xDeg))
                   * AGZ::Math::RM_Mat3f::RotateY(Degf(yDeg))
                   * AGZ::Math::RM_Mat3f::RotateZ(Degf(zDeg));

    auto rot = [&](int offset, void(*f)(const AGZ::Math::RM_Mat3f&, float*))
    {
        for(int c = 0; c < 3; ++c)
            f(rotateMat, &channels[c][offset]);
    };

    if(SHOrder >= 1) rot(0, &AGZ::Math::SH::RotateSH_L0<float>);
    if(SHOrder >= 2) rot(1, &AGZ::Math::SH::RotateSH_L1<float>);
    if(SHOrder >= 3) rot(4, &AGZ::Math::SH::RotateSH_L2<float>);
    if(SHOrder >= 4) rot(9, &AGZ::Math::SH::RotateSH_L3<float>);
    if(SHOrder >= 5) rot(16, &AGZ::Math::SH::RotateSH_L4<float>);

    for(int c = 0; c < 3; ++c)
    {
        for(int i = 0; i < SHC; ++i)
            coefs[i][c] = channels[c][i];
    }

    auto dir = absolute(std::filesystem::path(outFilename)).parent_path();
    if(!exists(dir))
        create_directory(dir);
    cnpy::npy_save(outFilename, &coefs[0][0], { size_t(SHC), 3 });
}
