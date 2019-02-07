#include <Atrc/ModelViewer/ResourceManagement/EntityCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/FilmFilterCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/FresnelCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/MaterialCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/GeometryCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/TextureCreator.h>

void RegisterResourceCreators(ResourceManager &rscMgr)
{
    RegisterEntityCreators(rscMgr);
    RegisterFilmFilterCreators(rscMgr);
    RegisterFresnelCreators(rscMgr);
    RegisterMaterialCreators(rscMgr);
    RegisterGeometryCreators(rscMgr);
    RegisterTextureCreators(rscMgr);
}
