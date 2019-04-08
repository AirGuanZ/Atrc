#include <Atrc/Editor/SH2DLightScriptExporter.h>

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

std::string SH2DLightScriptExporter::Export(const Atrc::Editor::ILight *light, SceneExportingContext &ctx, int SHOrder, int N) const
{
    ctx.ClearString();

    ctx.AddLine("workspace = \"@", relative(ctx.workspaceDirectory, ctx.scriptDirectory).string(), "\";");

    ctx.AddLine();

    ctx.AddLine("SHOrder = ", SHOrder, ";");
    ctx.AddLine("N = ", N, ";");

    ctx.AddLine();

    ctx.AddLine("light = ", light->Export(ctx.workspaceDirectory), ";");

    return ctx.GetString();
}
