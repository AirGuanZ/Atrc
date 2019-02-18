#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/LauncherScriptExporter.h>
#include <Atrc/Mgr/Parser.h>

LauncherScriptExporter::LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx)
    : rscMgr_(rscMgr), ctx_(ctx)
{
    
}

namespace
{
    template<typename TResource>
    void ExportResourcesInPool(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx)
    {
        ctx.AddLine(TResource::GetPoolName(), " = {");
        ctx.IncIndent();
        for(auto &rsc : rscMgr.GetPool<TResource>())
            IResource::ExportSubResource(rsc->GetName(), rscMgr, ctx, *rsc);
        ctx.DecIndent();
        ctx.AddLine("};");
    }
}

std::string LauncherScriptExporter::Export() const
{
    ctx_.ClearString();

    ctx_.AddLine("workspace = \"@", ctx_.workspaceDirectory, "\";");

    ctx_.AddLine();

    ctx_.AddLine("film = {");
    ctx_.IncIndent();
    ctx_.AddLine("size = (", ctx_.outputFilmSize.x, ", ", ctx_.outputFilmSize.y, ");");
    if(ctx_.filmFilter)
        IResource::ExportSubResource("filter", rscMgr_, ctx_, *ctx_.filmFilter);
    else
        Global::ShowNormalMessage("film filter is unspecified");
    ctx_.DecIndent();
    ctx_.AddLine("};");

    ctx_.AddLine();

    ctx_.AddLine("pool = {");
    ctx_.IncIndent();
    ExportResourcesInPool<CameraInstance>  (rscMgr_, ctx_);
    ExportResourcesInPool<EntityInstance>  (rscMgr_, ctx_);
    ExportResourcesInPool<GeometryInstance>(rscMgr_, ctx_);
    ExportResourcesInPool<LightInstance>   (rscMgr_, ctx_);
    ExportResourcesInPool<MaterialInstance>(rscMgr_, ctx_);
    ExportResourcesInPool<TextureInstance> (rscMgr_, ctx_);
    ctx_.DecIndent();
    ctx_.AddLine("};");

    ctx_.AddLine();

    if(ctx_.sampler)
    {
        IResource::ExportSubResource("sampler", rscMgr_, ctx_, *ctx_.sampler);
        ctx_.AddLine();
    }
    else
        Global::ShowNormalMessage("sampler is unspecified");

    if(ctx_.camera)
    {
        IResource::ExportSubResourceAsReference("camera", rscMgr_, ctx_, *ctx_.camera);
        ctx_.AddLine();
    }
    else
        Global::ShowNormalMessage("camera is unspecified");

    if(ctx_.renderer)
    {
        IResource::ExportSubResource("renderer", rscMgr_, ctx_, *ctx_.renderer);
        ctx_.AddLine();
    }
    else
        Global::ShowNormalMessage("renderer is unspecified");

    ctx_.AddLine("outputFilename = \"", ctx_.outputFilename, "\";");

    ctx_.AddLine();

    ctx_.AddLine("reporter = { type = Default; };");

    ctx_.AddLine();

    ctx_.AddLine("entities = (");
    for(auto &ent : rscMgr_.GetPool<EntityInstance>())
    {
        ctx_.AddLine("{");
        ctx_.IncIndent();
        ent->ExportAsReference<EntityInstance>(rscMgr_, ctx_);
        ctx_.DecIndent();
        ctx_.AddLine("},");
    }
    ctx_.AddLine(");");
    ctx_.AddLine();

    ctx_.AddLine("lights = (");
    for(auto &ent : rscMgr_.GetPool<LightInstance>())
    {
        ctx_.AddLine("{");
        ctx_.IncIndent();
        ent->ExportAsReference<LightInstance>(rscMgr_, ctx_);
        ctx_.DecIndent();
        ctx_.AddLine("},");
    }
    ctx_.AddLine(");");

    return ctx_.GetString();
}

void LauncherScriptImporter::Import(const AGZ::ConfigGroup &root, EditorData *data, std::string_view scriptPath)
{
    ImportFromPool<CameraInstance>  (root, data->rscMgr);
    ImportFromPool<EntityInstance>  (root, data->rscMgr);
    ImportFromPool<GeometryInstance>(root, data->rscMgr);
    ImportFromPool<LightInstance>   (root, data->rscMgr);
    ImportFromPool<MaterialInstance>(root, data->rscMgr);
    ImportFromPool<TextureInstance> (root, data->rscMgr);

    {
        auto &cameraGroup = GetFinalNonReferenceParam(root, root["camera"]);
        data->defaultRenderingCamera.Import(cameraGroup);
    }

    data->filmSize = Atrc::Mgr::Parser::ParseVec2i(root["film.size"]);
    {
        auto filmFilter = GetResourceInstance<FilmFilterInstance>(data->rscMgr, root, root["film.filter"]);
        data->filmFilterSlot.SetInstance(filmFilter);
    }
    {
        auto sampler = GetResourceInstance<SamplerInstance>(data->rscMgr, root, root["sampler"]);
        data->samplerSlot.SetInstance(sampler);
    }
    {
        auto renderer = GetResourceInstance<RendererInstance>(data->rscMgr, root, root["renderer"]);
        data->rendererSlot.SetInstance(renderer);
    }
    {
        auto script = relative(std::filesystem::path(scriptPath));
        data->scriptSlot.SetFilename(script.string());
        auto workspaceStr = root["workspace"].AsValue();
        std::filesystem::path workspace;
        if(AGZ::StartsWith(workspaceStr, "@"))
            workspace = relative(workspaceStr.substr(1), script);
        else
            workspace = workspaceStr;
        data->workspaceSlot.SetFilename(workspace.string());
    }
    std::strcpy(data->outputFilenameBuf.data(), root["outputFilename"].AsValue().c_str());
}
