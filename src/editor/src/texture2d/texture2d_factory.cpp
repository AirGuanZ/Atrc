#include <agz/editor/texture2d/constant2d.h>
#include <agz/editor/texture2d/image2d.h>
#include <agz/editor/texture2d/hdr.h>
#include <agz/editor/texture2d/range.h>

#include <agz/editor/texture2d/texture2d_factory.h>

AGZ_EDITOR_BEGIN

void init_texture2d_factory(Texture2DWidgetFactory &factory)
{
    factory.add_creator(newBox<Constant2DCreator>());
    factory.add_creator(newBox<Image2DCreator>());
    factory.add_creator(newBox<HDRWidgetCreator>());
    factory.add_creator(newBox<RangeWidgetCreator>());
}

AGZ_EDITOR_END
