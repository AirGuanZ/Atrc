#include <Atrc/ModelViewer/ResourceManagement/FilmFilterCreator.h>

namespace
{
    class BoxFilterInstance : public FilmFilterInstance
    {
        float radiusX_ = 0.5f;
        float radiusY_ = 0.5f;

    public:

        using FilmFilterInstance::FilmFilterInstance;

        void Display(ResourceManager &rscMgr) override
        {
            ImGui::InputFloat("radius.x", &radiusX_);
            ImGui::InputFloat("radius.y", &radiusY_);
        }
    };
}

void RegisterFilmFilterCreators(ResourceManager &rscMgr)
{
    static const BoxFilmterCreator iBoxFilmterCreator;
    rscMgr.AddCreator(&iBoxFilmterCreator);
}

std::shared_ptr<FilmFilterInstance> BoxFilmterCreator::Create(std::string name) const
{
    return std::make_shared<BoxFilterInstance>(std::move(name));
}
