#include <Atrc/Mgr/BuiltinCreator/CameraCreator.h>
#include <Atrc/Mgr/BuiltinCreator/EntityCreator.h>
#include <Atrc/Mgr/BuiltinCreator/FilmFilterCreator.h>
#include <Atrc/Mgr/BuiltinCreator/GeometryCreator.h>
#include <Atrc/Mgr/BuiltinCreator/LightCreator.h>
#include <Atrc/Mgr/BuiltinCreator/MaterialCreator.h>
#include <Atrc/Mgr/BuiltinCreator/MediumCreator.h>
#include <Atrc/Mgr/BuiltinCreator/PathTracingIntegratorCreator.h>
#include <Atrc/Mgr/BuiltinCreator/PostProcessorCreator.h>
#include <Atrc/Mgr/BuiltinCreator/RendererCreator.h>
#include <Atrc/Mgr/BuiltinCreator/ReporterCreator.h>
#include <Atrc/Mgr/BuiltinCreator/SamplerCreator.h>
#include <Atrc/Mgr/BuiltinCreator/TextureCreator.h>
#include <Atrc/Mgr/BuiltinCreatorRegister.h>

namespace Atrc::Mgr
{

void RegisterBuiltinCreators(Context &context)
{
    AGZ_HIERARCHY_TRY

    RegisterBuiltinCameraCreators(context);
    RegisterBuiltinEntityCreators(context);
    RegisterBuiltinFilmFilterCreators(context);
    RegisterBuiltinGeometryCreators(context);
    RegisterBuiltinLightCreators(context);
    RegisterBuiltinMaterialCreators(context);
    RegisterBuiltinMediumCreators(context);
    RegisterBuiltinPathTracingIntegratorCreators(context);
    RegisterBuiltinPostProcessorCreators(context);
    RegisterBuiltinRendererCreators(context);
    RegisterBuiltinReportCreators(context);
    RegisterBuiltinSamplerCreators(context);
    RegisterBuiltinTextureCreators(context);

    AGZ_HIERARCHY_WRAP("In registering builtin object creators")
}

} // namespace Atrc::Mgr
