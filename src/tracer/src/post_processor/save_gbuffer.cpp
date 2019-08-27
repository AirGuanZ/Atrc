#include <agz/tracer/core/post_processor.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/file.h>
#include <agz/utility/image.h>

AGZ_TRACER_BEGIN

class SaveGBufferToPNG : public PostProcessor
{
    std::string albedo_filename_;
    std::string normal_filename_;
    std::string depth_filename_;

    static void save_albedo(const std::string &filename, AlbedoBuffer &albedo)
    {
        file::create_directory_for_file(filename);
        AGZ_LOG0("saving gbuffer::albedo to ", filename);
        img::save_rgb_to_png_file(filename, albedo.get_data().map(math::to_color3b<real>));
    }

    static void save_normal(const std::string &filename, NormalBuffer &normal)
    {
        file::create_directory_for_file(filename);
        texture::texture2d_t<Spectrum> imgf(normal.height(), normal.width());
        for(int y = 0; y < imgf.height(); ++y)
        {
            for(int x = 0; x < imgf.width(); ++x)
            {
                Spectrum nor(normal(y, x).x, normal(y, x).y, normal(y, x).z);
                imgf(y, x) = (Spectrum(1) + nor) * real(0.5);
            }
        }

        AGZ_LOG0("saving gbuffer::normal to ", filename);
        img::save_rgb_to_png_file(filename, imgf.get_data().map(math::to_color3b<real>));
    }

    static void save_depth(const std::string &filename, DepthBuffer &depth)
    {
        file::create_directory_for_file(filename);
        real min_depth = std::numeric_limits<real>::max(), max_depth = std::numeric_limits<real>::min();
        for(int y = 0; y < depth.height(); ++y)
        {
            for(int x = 0; x < depth.width(); ++x)
            {
                if(depth(y, x) < 0)
                    continue;
                min_depth = (std::min)(min_depth, depth(y, x));
                max_depth = (std::max)(max_depth, depth(y, x));
            }
        }

        if(min_depth == std::numeric_limits<real>::max())
            min_depth = EPS;
        if(max_depth == std::numeric_limits<real>::min())
            max_depth = 1;

        real delta = max_depth > min_depth ? max_depth - min_depth : 1;
        real rcv_delta = 1 / delta;
        texture::texture2d_t<uint8_t> imgu8(depth.height(), depth.width());
        for(int y = 0; y < depth.height(); ++y)
        {
            for(int x = 0; x < depth.width(); ++x)
            {
                if(depth(y, x) < 0)
                {
                    imgu8(y, x) = 0;
                    continue;
                }
                real f = (depth(y, x) - min_depth) * rcv_delta;
                uint8_t pixel = static_cast<uint8_t>(math::clamp<real>(f, 0, 1) * 255);
                imgu8(y, x) = pixel;
            }
        }

        AGZ_LOG0("saving gbuffer::depth to ", filename);
        img::save_gray_to_png_file(filename, imgu8.get_data());
    }

public:

    using PostProcessor::PostProcessor;

    static std::string description()
    {
        return R"___(
save_gbuffer_to_png [PostProcessor]
    albedo [string] (optional; defaultly set to "") save albedo to ?
    normal [string] (optional; defaultly set to "") save normal to ?
    depth  [string] (optional; defaultly set to "") save depth  to ?
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        if(auto node = params.find_child("albedo"))
            albedo_filename_ = init_ctx.path_mgr->get(node->as_value().as_str());
        if(auto node = params.find_child("normal"))
            normal_filename_ = init_ctx.path_mgr->get(node->as_value().as_str());
        if(auto node = params.find_child("depth"))
            depth_filename_ = init_ctx.path_mgr->get(node->as_value().as_str());

        AGZ_HIERARCHY_WRAP("in initializing save_gbuffer_to_image post processor")
    }

    void process(texture::texture2d_t<Spectrum> &, GBuffer &gbuffer) override
    {
        if(!albedo_filename_.empty() && gbuffer.albedo)
            save_albedo(albedo_filename_, *gbuffer.albedo);
        if(!normal_filename_.empty() && gbuffer.normal)
            save_normal(normal_filename_, *gbuffer.normal);
        if(!depth_filename_.empty() && gbuffer.depth)
            save_depth(depth_filename_, *gbuffer.depth);
    }
};

AGZT_IMPLEMENTATION(PostProcessor, SaveGBufferToPNG, "save_gbuffer_to_png")

AGZ_TRACER_END
