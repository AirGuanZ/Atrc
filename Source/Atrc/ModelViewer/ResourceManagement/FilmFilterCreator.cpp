#include <Atrc/ModelViewer/ResourceManagement/FilmFilterCreator.h>

namespace
{
    class BoxFilterInstance : public FilmFilterInstance
    {
        float sidelen_ = 1;

    public:

        using FilmFilterInstance::FilmFilterInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("sidelen", &sidelen_);
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Box;\n";
            sst << ctx.Indent() << "sidelen = " << std::to_string(sidelen_) << ";\n";
        }
    };

    class GaussianFilterInstance : public FilmFilterInstance
    {
        float radius_ = 0.7f;
        float alpha_ = 2;

    public:

        using FilmFilterInstance::FilmFilterInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("radius.x", &radius_);
            ImGui::InputFloat("alpha", &alpha_);
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Box;\n";
            sst << ctx.Indent() << "radius = " << std::to_string(radius_) << ";\n";
            sst << ctx.Indent() << "alpha = " << std::to_string(alpha_) << ";\n";
        }
    };
}

void RegisterFilmFilterCreators(ResourceManager &rscMgr)
{
    static const BoxFilterCreator iBoxFilterCreator;
    static const GaussianFilterCreator iGaussianFilterCreator;
    rscMgr.AddCreator(&iBoxFilterCreator);
    rscMgr.AddCreator(&iGaussianFilterCreator);
}

std::shared_ptr<FilmFilterInstance> BoxFilterCreator::Create(std::string name) const
{
    return std::make_shared<BoxFilterInstance>(std::move(name));
}

std::shared_ptr<FilmFilterInstance> GaussianFilterCreator::Create(std::string name) const
{
    return std::make_shared<GaussianFilterInstance>(std::move(name));
}
