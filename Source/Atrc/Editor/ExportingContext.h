#pragma once

#include <sstream>

#include <Atrc/Editor/Camera.h>
#include <Atrc/Editor/TransformController.h>

class LauncherScriptExportingContext
{
    size_t indent_;
    std::string indentStr_;

    std::stringstream sst_;

    template<typename TStr>
    void AddLineAux(TStr &&str)
    {
        sst_ << std::forward<TStr>(str);
    }

    template<typename TStr0, typename TStr1, typename...Others>
    void AddLineAux(TStr0 &&str0, TStr1 &&str1, Others&&...others)
    {
        AddLineAux(std::forward<TStr0>(str0));
        AddLineAux(std::forward<TStr1>(str1), std::forward<Others>(others)...);
    }

public:

    const DefaultRenderingCamera * const activeCamera;
    const std::string workspaceDirectory;
    const std::string scriptDirectory;

    const Transform *entityTransform;

    LauncherScriptExportingContext(const DefaultRenderingCamera *activeCamera, std::string workspace, std::string scriptDir)
        : indent_(0),
          activeCamera(activeCamera), workspaceDirectory(std::move(workspace)), scriptDirectory(std::move(scriptDir)),
          entityTransform(nullptr)
    {
        AGZ_ASSERT(activeCamera);
    }

    void IncIndent()
    {
        indentStr_ = std::string(4 * ++indent_, ' ');
    }

    void DecIndent()
    {
        indentStr_ = std::string(4 * --indent_, ' ');
    }

    std::string GetString() const
    {
        return sst_.str();
    }

    template<typename...TStrs>
    void AddLine(TStrs&&...strs)
    {
        sst_ << indentStr_;
        AddLineAux(std::forward<TStrs>(strs)...);
        sst_ << std::endl;
    }
};
