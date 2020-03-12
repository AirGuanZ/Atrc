#include <agz/tracer/core/post_processor.h>
#include <agz/utility/misc.h>

AGZ_TRACER_BEGIN

class Flip : public PostProcessor
{
    bool vertically_   = false;
    bool horizontally_ = false;

public:

    Flip(bool vertically, bool horizontally)
    {
        vertically_ = vertically;
        horizontally_ = horizontally;
    }

    void process(RenderTarget &render_target) override
    {
        if(vertically_)
        {
            render_target.image   = render_target.image  .flip_vertically();
            render_target.albedo  = render_target.albedo .flip_vertically();
            render_target.normal  = render_target.normal .flip_vertically();
            render_target.denoise = render_target.denoise.flip_vertically();
        }

        if(horizontally_)
        {
            render_target.image   = render_target.image  .flip_horizontally();
            render_target.albedo  = render_target.albedo .flip_horizontally();
            render_target.normal  = render_target.normal .flip_horizontally();
            render_target.denoise = render_target.denoise.flip_horizontally();
        }
    }
};

RC<PostProcessor> create_film_flipper(
    bool vertically, bool horizontally)
{
    return newRC<Flip>(vertically, horizontally);
}

AGZ_TRACER_END
