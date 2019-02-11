#pragma once

#include <Atrc/Editor/LauncherScriptExporter.h>
#include "Global.h"

LauncherScriptExporter::LauncherScriptExporter(ResourceManager &rscMgr, LauncherScriptExportingContext &ctx)
    : rscMgr_(rscMgr), ctx_(ctx)
{
    
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

    if(ctx_.sampler)
    {
        IResource::ExportSubResource("sampler", rscMgr_, ctx_, *ctx_.sampler);
        ctx_.AddLine();
    }
    else
        Global::ShowNormalMessage("sampler is unspecified");

    if(ctx_.camera)
    {
        IResource::ExportSubResource("camera", rscMgr_, ctx_, *ctx_.camera);
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
        ent->Export(rscMgr_, ctx_);
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
        ent->Export(rscMgr_, ctx_);
        ctx_.DecIndent();
        ctx_.AddLine("},");
    }
    ctx_.AddLine(");");

    return ctx_.GetString();
}
