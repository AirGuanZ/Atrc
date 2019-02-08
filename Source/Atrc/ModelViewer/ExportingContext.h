#pragma once

#include <Atrc/ModelViewer/Camera.h>
#include <Atrc/ModelViewer/TransformController.h>

class ExportingContext
{
public:

    const Camera *activeCamera = nullptr;
    std::string workspaceDirectory;
    std::string scriptDirectory;

    size_t indent = 0;

    const Transform *entityTransform = nullptr;

    std::string Indent() const
    {
        return std::string(4 * indent, ' ');
    }

    bool IsComplete() const noexcept
    {
        return activeCamera;
    }
};
