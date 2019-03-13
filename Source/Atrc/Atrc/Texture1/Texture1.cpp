#include <Atrc/Atrc/Texture1/Texture1.h>

#include <Atrc/Atrc/Texture1/Constant1.h>

void RegisterTexture1Creators(ResourceCreatorManager<Texture1Instance> &mgr)
{
    static const Core2Texture1Creator<Constant1Core> iConstant1;
    mgr.AddCreator(&iConstant1);
}
