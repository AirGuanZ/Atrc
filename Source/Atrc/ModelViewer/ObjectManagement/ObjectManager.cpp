#include <Atrc/ModelViewer/ObjectManagement/FresnelCreator.h>
#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>
#include <Atrc/ModelViewer/ObjectManagement/TextureCreator.h>

void RegisterObjectCreators(ObjectManager &objMgr)
{
    RegisterFresnelCreators(objMgr);
    RegisterMaterialCreators(objMgr);
    RegisterTextureCreators(objMgr);
}
