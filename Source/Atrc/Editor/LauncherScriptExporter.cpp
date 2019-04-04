#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/LauncherScriptExporter.h>
#include <Atrc/Mgr/Parser.h>

namespace
{
    template<typename TResource>
    void ExportResourcesInPool(ResourceManager &rscMgr, SceneExportingContext &ctx)
    {
        ctx.AddLine(TResource::GetPoolName(), " = {");
        ctx.IncIndent();
        for(auto &rsc : rscMgr.GetPool<TResource>())
            ResourceInstance::ExportSubResource(rsc->GetName(), rscMgr, ctx, *rsc);
        ctx.DecIndent();
        ctx.AddLine("};");
    }
}

std::string LauncherScriptExporter::Export(
    ResourceManager &rscMgr, SceneExportingContext &ctx,
    const RendererInstance *renderer,
    const IFilmFilter *filmFilter,
    const SamplerInstance    *sampler,
    const Vec2i &outputFilmSize,
    const std::string &outputFilename) const
{
    ctx.ClearString();

    ctx.AddLine("workspace = \"@", relative(ctx.workspaceDirectory, ctx.scriptDirectory).string(), "\";");

    ctx.AddLine();

    ctx.AddLine("film = {");
    ctx.IncIndent();
    ctx.AddLine("size = (", outputFilmSize.x, ", ", outputFilmSize.y, ");");
    if(filmFilter)
        ctx.AddLine("filter = ", filmFilter->Export(), ";");
    else
        Global::ShowNormalMessage("film filter is unspecified");
    ctx.DecIndent();
    ctx.AddLine("};");

    ctx.AddLine();

    ctx.AddLine("pool = {");
    ctx.IncIndent();
    ExportResourcesInPool<CameraInstance>  (rscMgr, ctx);
    ExportResourcesInPool<EntityInstance>  (rscMgr, ctx);
    ExportResourcesInPool<GeometryInstance>(rscMgr, ctx);
    ExportResourcesInPool<LightInstance>   (rscMgr, ctx);
    ExportResourcesInPool<MaterialInstance>(rscMgr, ctx);
    ExportResourcesInPool<TextureInstance> (rscMgr, ctx);
    ctx.DecIndent();
    ctx.AddLine("};");

    ctx.AddLine();

    if(sampler)
    {
        ResourceInstance::ExportSubResource("sampler", rscMgr, ctx, *sampler);
        ctx.AddLine();
    }
    else
        Global::ShowNormalMessage("sampler is unspecified");

    if(ctx.camera)
    {
        ResourceInstance::ExportSubResourceAsReference("camera", rscMgr, ctx, *ctx.camera);
        ctx.AddLine();
    }
    else
        Global::ShowNormalMessage("camera is unspecified");

    if(renderer)
    {
        ResourceInstance::ExportSubResource("renderer", rscMgr, ctx, *renderer);
        ctx.AddLine();
    }
    else
        Global::ShowNormalMessage("renderer is unspecified");

    ctx.AddLine("outputFilename = \"", outputFilename, "\";");

    ctx.AddLine();

    ctx.AddLine("reporter = { type = Default; };");

    ctx.AddLine();

    ctx.AddLine("entities = (");
    for(auto &ent : rscMgr.GetPool<EntityInstance>())
    {
        ctx.AddLine("{");
        ctx.IncIndent();
        ent->ExportAsReference<EntityInstance>(rscMgr, ctx);
        ctx.DecIndent();
        ctx.AddLine("},");
    }
    ctx.AddLine(");");
    ctx.AddLine();

    ctx.AddLine("lights = (");
    for(auto &ent : rscMgr.GetPool<LightInstance>())
    {
        ctx.AddLine("{");
        ctx.IncIndent();
        ent->ExportAsReference<LightInstance>(rscMgr, ctx);
        ctx.DecIndent();
        ctx.AddLine("},");
    }
    ctx.AddLine(");");

    return ctx.GetString();
}

void LauncherScriptImporter::Import(const AGZ::ConfigGroup &root, EditorData *data, std::string_view scriptFilename)
{
    ResourceInstance::ImportContext ctx;

    {
        auto scriptDir = absolute(std::filesystem::path(scriptFilename).parent_path());
        data->scriptFilename.SetFilename(relative(std::filesystem::path(scriptFilename)).string());
        auto workspaceStr = root["workspace"].AsValue();
        std::filesystem::path workspace;
        if(AGZ::StartsWith(workspaceStr, "@"))
            workspace = absolute(scriptDir / workspaceStr.substr(1));
        else if(AGZ::StartsWith(workspaceStr, "$"))
            workspace = absolute(std::filesystem::path(workspaceStr.substr(1)));
        else
            workspace = absolute(std::filesystem::path(workspaceStr));
        data->workspaceDir.SetFilename(relative(workspace).string());
        
        ctx.scriptPath = scriptDir;
        ctx.workspacePath = absolute(workspace);
    }

    data->rscMgr.GetPool<CameraInstance>().Clear();
    data->rscMgr.GetPool<EntityInstance>().Clear();
    data->rscMgr.GetPool<GeometryInstance>().Clear();
    data->rscMgr.GetPool<LightInstance>().Clear();
    data->rscMgr.GetPool<MaterialInstance>().Clear();
    data->rscMgr.GetPool<TextureInstance>().Clear();

    ImportFromPool<CameraInstance>  (root, data->rscMgr, ctx);
    ImportFromPool<EntityInstance>  (root, data->rscMgr, ctx);
    ImportFromPool<GeometryInstance>(root, data->rscMgr, ctx);
    ImportFromPool<LightInstance>   (root, data->rscMgr, ctx);
    ImportFromPool<MaterialInstance>(root, data->rscMgr, ctx);
    ImportFromPool<TextureInstance> (root, data->rscMgr, ctx);

    {
        auto &cameraGroup = GetFinalNonReferenceParam(root, root["camera"]);
        data->defaultRenderingCamera.Import(cameraGroup);

        auto cam = GetResourceInstance<CameraInstance>(data->rscMgr, root, root["camera"], ctx);
        auto &camPool = data->rscMgr.GetPool<CameraInstance>();
        if(camPool.Find(cam))
            camPool.SetSelectedInstance(cam);
    }

    data->filmSize = Atrc::Mgr::Parser::ParseVec2i(root["film.size"]);
    {
        auto filter = RF.Get<IFilmFilter>()[root["film.filter.type"].AsValue()].Create();
        filter->Load(root["film.filter"].AsGroup());
        data->filmFilter->SetResource(filter);
    }
    {
        auto sampler = GetResourceInstance<SamplerInstance>(data->rscMgr, root, root["sampler"], ctx);
        data->samplerSlot.SetInstance(sampler);
    }
    {
        auto renderer = GetResourceInstance<RendererInstance>(data->rscMgr, root, root["renderer"], ctx);
        data->rendererSlot.SetInstance(renderer);
    }
    std::strcpy(data->outputFilenameBuf.data(), root["outputFilename"].AsValue().c_str());
}
