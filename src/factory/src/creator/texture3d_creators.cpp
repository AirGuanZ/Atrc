#include <fstream>

#include <agz/factory/creator/texture3d_creators.h>
#include <agz/factory/utility/texture3d_loader.h>
#include <agz/tracer/create/texture3d.h>

AGZ_TRACER_FACTORY_BEGIN

namespace
{
    Texture3DCommonParams parse_common_params(const ConfigGroup &params)
    {
        Texture3DCommonParams ret;
        ret.from_params(params);
        return ret;
    }

    class Constant3DCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "constant";
        }

        RC<Texture3D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const auto common_params = parse_common_params(params);
            const Spectrum texel = params.child_spectrum("texel");
            return create_constant3d_texture(common_params, texel);
        }
    };

    class ImageTexture3DCreator : public Creator<Texture3D>
    {
    public:

        std::string name() const override
        {
            return "image3d";
        }

        std::shared_ptr<Texture3D> create(
            const ConfigGroup &params, CreatingContext &context) const override
        {
            const Texture3DCommonParams common_params = parse_common_params(params);
            const std::string sampling_strategy = params.child_str_or(
                "sampler", "linear");
            const bool use_linear_sampler = sampling_strategy == "linear";

            // format: real/spec/gray8/rgb8
            const std::string format = params.child_str("format");
            
            if(params.find_child("ascii_filename"))
            {
                const std::string filename = context.path_mapper->map(
                    params.child_str("ascii_filename"));
                std::ifstream fin(filename, std::ios::in);
                if(!fin)
                {
                    throw ObjectConstructionException(
                        "failed to open file: " + filename);
                }

                if(format == "real")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_real_from_ascii(fin)),
                        use_linear_sampler);
                }

                if(format == "spec")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_spec_from_ascii(fin)),
                        use_linear_sampler);
                }

                if(format == "gray8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint8_from_ascii(fin)),
                        use_linear_sampler);
                }

                if(format == "rgb8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint24_from_ascii(fin)),
                        use_linear_sampler);
                }

                throw ObjectConstructionException(
                    "unknown image3d format: " + format);
            }

            if(params.find_child("binary_filename"))
            {
                const std::string filename = context.path_mapper->map(
                    params.child_str("binary_filename"));
                std::ifstream fin(filename, std::ios::in | std::ios::binary);
                if(!fin)
                {
                    throw ObjectConstructionException(
                        "failed to open file: " + filename);
                }

                if(format == "real")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_real_from_binary(fin)),
                        use_linear_sampler);
                }

                if(format == "spec")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_spec_from_binary(fin)),
                        use_linear_sampler);
                }

                if(format == "gray8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint8_from_binary(fin)),
                        use_linear_sampler);
                }

                if(format == "rgb8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint24_from_binary(fin)),
                        use_linear_sampler);
                }

                throw ObjectConstructionException(
                    "unknown image3d format: " + format);
            }

            if(params.find_child("image_filenames"))
            {
                const auto &arr = params.child_array("image_filenames");
                std::vector<std::string> filenames(arr.size());
                for(size_t i = 0; i < arr.size(); ++i)
                    filenames[i] = arr.at_str(i);

                if(format == "real")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_real_from_images(
                            filenames.data(),
                            int(filenames.size()),
                            *context.path_mapper)),
                        use_linear_sampler);
                }

                if(format == "spec")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_spec_from_images(
                            filenames.data(),
                            int(filenames.size()),
                            *context.path_mapper)),
                        use_linear_sampler);
                }

                if(format == "gray8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint8_from_images(
                            filenames.data(),
                            int(filenames.size()),
                            *context.path_mapper)),
                        use_linear_sampler);
                }

                if(format == "rgb8")
                {
                    return create_image3d(
                        common_params,
                        toRC(texture3d_load::load_uint24_from_images(
                            filenames.data(),
                            int(filenames.size()),
                            *context.path_mapper)),
                        use_linear_sampler);
                }

                throw ObjectConstructionException(
                    "unknown image3d format: " + format);
            }

            throw ObjectConstructionException("input filename is unspecified");
        }
    };

}

void initialize_texture3d_factory(Factory<Texture3D> &factory)
{
    factory.add_creator(newBox<Constant3DCreator>());
    factory.add_creator(newBox<ImageTexture3DCreator>());
}

AGZ_TRACER_FACTORY_END
