#include <Atrc/Mgr/BuiltinCreator/CameraCreator.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>
#include <Atrc/Mgr/BuiltinCreator/FilmFilterCreator.h>
#include <Atrc/Mgr/BuiltinCreator/GeometryCreator.h>
#include <Atrc/Mgr/BuiltinCreator/LightCreator.h>
#include <Atrc/Mgr/BuiltinCreator/MaterialCreator.h>
#include <Atrc/Mgr/BuiltinCreator/RendererCreator.h>
#include <Atrc/Mgr/BuiltinCreator/SamplerCreator.h>
#include <Atrc/Mgr/BuiltinCreator/TextureCreator.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>

namespace Atrc::Mgr
{

void RegisterBuiltinCreators(Context &context)
{
    ATRC_MGR_TRY
    {
        RegisterBuiltinCameraCreators    (context);
        RegisterBuiltinEntityCreators    (context);
        RegisterBuiltinFilmFilterCreators(context);
        RegisterBuiltinGeometryCreators  (context);
        RegisterBuiltinLightCreators     (context);
        RegisterBuiltinMaterialCreators  (context);
        RegisterBuiltinRendererCreators  (context);
        RegisterBuiltinSamplerCreators   (context);
        RegisterBuiltinTextureCreators   (context);
    }
    ATRC_MGR_CATCH_AND_RETHROW("In registering builtin object creators")
}

} // namespace Atrc::Mgr
