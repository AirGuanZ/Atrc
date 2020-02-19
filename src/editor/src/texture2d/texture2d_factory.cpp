#include <agz/editor/texture2d/constant2d.h>
#include <agz/editor/texture2d/image2d.h>
#include <agz/editor/texture2d/hdr.h>
#include <agz/editor/texture2d/range.h>

#include <agz/editor/texture2d/texture2d_factory.h>

AGZ_EDITOR_BEGIN

void init_texture2d_factory(Texture2DWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<Constant2DCreator>());
    factory.add_creator(std::make_unique<Image2DCreator>());
    factory.add_creator(std::make_unique<HDRWidgetCreator>());
    factory.add_creator(std::make_unique<RangeWidgetCreator>());
}

AGZ_EDITOR_END
