#include <Atrc/EnvLight/Internal.h>
#include <Lib/cnpy/cnpy.h>

using namespace AGZ::Math;

std::pair<int, std::vector<Vec3f>> LoadSHFromNPY(const std::string &npyFilename)
{
    auto arr = cnpy::npy_load(npyFilename);
    if(arr.shape.size() != 2 || arr.shape[1] != 3 || arr.word_size != sizeof(float))
        return { 0, { } };

    int SHOrder;
    switch(arr.shape[0])
    {
    case 1: SHOrder = 1; break;
    case 4: SHOrder = 2; break;
    case 9: SHOrder = 3; break;
    case 16: SHOrder = 4; break;
    case 25: SHOrder = 5; break;
    default: return { 0, { } };
    }

    int SHC = SHOrder * SHOrder;
    auto data = arr.data<float>();
    std::vector<Vec3f> coefs;

    if(arr.fortran_order)
    {
        // ”–j––£¨SHC¡–
        coefs.resize(SHC);
        for(int j = 0; j < 3; ++j)
        {
            for(int i = 0; i < SHC; ++i)
                coefs[i][j] = data[j * SHC + i];
        }
    }
    else
    {
        coefs.reserve(SHC);
        for(int i = 0, j = 0; i < SHC; ++i, j += 3)
        {
            Vec3f c(data[j], data[j + 1], data[j + 2]);
            coefs.push_back(c);
        }
    }

    return { SHOrder, std::move(coefs) };
}
