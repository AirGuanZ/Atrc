#include <agz/editor/texture2d/constant_texture2d.h>

AGZ_EDITOR_BEGIN

class ConstantTexture2DCreator : public Texture2DWidgetCreator
{
public:

    QString name() const override
    {
        return "Constant";
    }

    ResourceWidget<tracer::Texture2D> *create_widget() const override
    {
        return new ConstantTexture2DWidget;
    }
};

void init_texture2d_factory(Texture2DWidgetFactory &factory)
{
    factory.add_creator(std::make_unique<ConstantTexture2DCreator>());
}

AGZ_EDITOR_END
