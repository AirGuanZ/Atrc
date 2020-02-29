#include <agz/editor/texture3d/constant3d.h>
#include <agz/editor/texture3d/gray_grid3d.h>
#include <agz/editor/texture3d/range3d.h>
#include <agz/editor/texture3d/spectrum_grid3d.h>

#include <agz/editor/texture3d/texture3d_factory.h>

AGZ_EDITOR_BEGIN

void init_texture3d_factory(Texture3DWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<Constant3DWidgetCreator>());
    factory.add_creator(std::make_unique<GrayGrid3DWidgetCreator>());
    factory.add_creator(std::make_unique<Range3DWidgetCreator>());
    factory.add_creator(std::make_unique<SpectrumGrid3DWidgetCreator>());
}

AGZ_EDITOR_END
