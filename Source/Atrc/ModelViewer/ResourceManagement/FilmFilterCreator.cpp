#include <Atrc/ModelViewer/ResourceManagement/FilmFilterCreator.h>

namespace
{
    class BoxFilterInstance : public FilmFilterInstance
    {
        float radiusX_ = 0.5f;
        float radiusY_ = 0.5f;

    public:

        using FilmFilterInstance::FilmFilterInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("radius.x", &radiusX_);
            ImGui::InputFloat("radius.y", &radiusY_);
        }
    };

    class GaussianFilterInstance : public FilmFilterInstance
    {
        float radiusX_ = 0.7f;
        float radiusY_ = 0.7f;
        float alpha_ = 2;

    public:

        using FilmFilterInstance::FilmFilterInstance;

        void Display(ResourceManager&) override
        {
            ImGui::InputFloat("radius.x", &radiusX_);
            ImGui::InputFloat("radius.y", &radiusY_);
            ImGui::InputFloat("alpha", &alpha_);
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
