#pragma once

#include <Atrc/ModelViewer/LauncherScriptExporter.h>

LauncherScriptExporter::LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx)
    : rscMgr_(rscMgr), ctx_(ctx)
{
    
}

std::string LauncherScriptExporter::Export() const
{
    ctx_.ClearString();

    ctx_.AddLine("workspace = @", ctx_.workspaceDirectory, ";");

    ctx_.AddLine();

    ctx_.AddLine("film = {");
    ctx_.IncIndent();
    ctx_.AddLine("size = (", ctx_.outputFilmSize.x, ", ", ctx_.outputFilmSize.y, ");");
    ctx_.filmFilter->Export(rscMgr_, ctx_);
    ctx_.DecIndent();
    ctx_.AddLine("};");

    ctx_.AddLine();

    IResource::ExportSubResource("sampler", rscMgr_, ctx_, *ctx_.sampler);

    ctx_.AddLine();

    IResource::ExportSubResource("camera", rscMgr_, ctx_, *ctx_.camera);

    ctx_.AddLine();

    IResource::ExportSubResource("renderer", rscMgr_, ctx_, *ctx_.renderer);

    ctx_.AddLine();

    ctx_.AddLine("outputFilename = ", ctx_.outputFilename, ";");

    ctx_.AddLine();

    ctx_.AddLine("reporter = { type = Default; };");

    ctx_.AddLine();

    ctx_.AddLine("entities = (");
    for(auto &ent : rscMgr_.GetPool<EntityInstance>())
    {
        ctx_.AddLine("{");
        ctx_.IncIndent();
        ent->Export(rscMgr_, ctx_);
        ctx_.DecIndent();
        ctx_.AddLine("},");
    }
    ctx_.AddLine(");");

    return ctx_.GetString();
}
