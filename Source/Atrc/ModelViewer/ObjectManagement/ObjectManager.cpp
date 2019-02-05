#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>
#include <Atrc/ModelViewer/ObjectManagement/ObjectManager.h>

void RegisterObjectCreators(ObjectManager &objMgr)
{
    RegisterMaterialCreators(objMgr);
}
