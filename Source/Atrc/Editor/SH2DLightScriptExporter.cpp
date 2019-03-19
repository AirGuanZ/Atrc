#include <Atrc/Editor/SH2DLightScriptExporter.h>

namespace
{
    template<typename TResource>
    void ExportResourcesInPool(ResourceManager &rscMgr, SceneExportingContext &ctx)
    {
        ctx.AddLine(TResource::GetPoolName(), " = {");
        ctx.IncIndent();
        for(auto &rsc : rscMgr.GetPool<TResource>())
            IResource::ExportSubResource(rsc->GetName(), rscMgr, ctx, *rsc);
        ctx.DecIndent();
        ctx.AddLine("};");
    }
}

std::string SH2DLightScriptExporter::Export(ResourceManager &rscMgr, SceneExportingContext &ctx, int SHOrder, int N) const
{
    ctx.ClearString();

    ctx.AddLine("workspace = \"@", relative(ctx.workspaceDirectory, ctx.scriptDirectory).string(), "\";");

    ctx.AddLine();

    ctx.AddLine("pool = {");
    ctx.IncIndent();
    ExportResourcesInPool<CameraInstance>(rscMgr, ctx);
    ExportResourcesInPool<EntityInstance>(rscMgr, ctx);
    ExportResourcesInPool<GeometryInstance>(rscMgr, ctx);
    ExportResourcesInPool<MaterialInstance>(rscMgr, ctx);
    ExportResourcesInPool<TextureInstance>(rscMgr, ctx);
    ctx.DecIndent();
    ctx.AddLine("};");

    ctx.AddLine();

    ctx.AddLine("SHOrder = ", SHOrder, ";");
    ctx.AddLine("N = ", N, ";");

    ctx.AddLine();

    auto &pool = rscMgr.GetPool<LightInstance>();
    auto beg = pool.begin(), end = pool.end();

    if(beg != end)
        IResource::ExportSubResource("light", rscMgr, ctx, **beg);

    return ctx.GetString();
}
