#include <agz/editor/texture3d/constant3d.h>
#include <agz/editor/texture3d/image3d.h>
#include <agz/editor/texture3d/range3d.h>

#include <agz/editor/texture3d/texture3d_factory.h>

AGZ_EDITOR_BEGIN

void init_texture3d_factory(Texture3DWidgetFactory &factory)
{
    factory.add_creator(newBox<Constant3DWidgetCreator>());
    factory.add_creator(newBox<Image3DWidgetCreator>());
    factory.add_creator(newBox<Range3DWidgetCreator>());
}

AGZ_EDITOR_END
