#include <vector>

#include <Atrc/Mgr/SceneBuilder.h>

namespace Atrc::Mgr
{

Scene SceneBuilder::Build(const ConfigGroup &root, Context &context)
{
    ATRC_MGR_TRY
    {
        auto camera = context.Create<Camera>(root["camera"]);

        std::vector<Entity*> entities;
        std::vector<Light*> lights;

        ATRC_MGR_TRY
        {
            auto &entArr = root["entities"].AsArray();
            for(auto ent : entArr)
                entities.push_back(context.Create<Entity>(*ent));
        }
        ATRC_MGR_CATCH_AND_RETHROW("In creating entities")

        ATRC_MGR_TRY
        {
            auto &lhtArr = root["lights"].AsArray();
            for(auto lht : lhtArr)
                lights.push_back(context.Create<Light>(*lht));
        }
        ATRC_MGR_CATCH_AND_RETHROW("In creating lights")

        std::vector<const Entity*> cEntities;
        std::vector<const Light*> cLights;

        for(auto ent : entities)
        {
            cEntities.push_back(ent);
            if(auto lht = ent->AsLight())
                lights.push_back(lht);
        }

        for(auto lht : lights)
            cLights.push_back(lht);

        Medium *globalMedium = nullptr;
        if(auto medNode = root.Find("globalMedium"))
            globalMedium = context.Create<Medium>(*medNode);

        Scene scene(
            cEntities.data(), cEntities.size(),
            cLights.data(), cLights.size(),
            camera, globalMedium);

        for(auto lht : lights)
            lht->PreprocessScene(scene);
        
        return scene;
    }
    ATRC_MGR_CATCH_AND_RETHROW("In creating Atrc scene")
}

} // namespace Atrc::Mgr
