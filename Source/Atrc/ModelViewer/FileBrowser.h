#pragma once

#include <filesystem>

#include <Atrc/ModelViewer/GL.h>

class FileBrowser
{
public:

    void SetLabel(std::string label);

    void SetTarget(bool selectDirectory);

    void SetCurrentDirectory(std::string_view dir = "");

    bool Display();

    const std::string &GetLabel() const noexcept;

    std::string GetResult(bool relative = true) const;

private:

    void UpdateCurrentUnits();

    std::string label_ = "default file browser";
    bool selectDirectory_ = false;

    std::string selectedName_;
    std::filesystem::path pwd_;

    struct Unit
    {
        bool isDir;
        std::string name;
    };

    std::vector<Unit> curUnits_;
};
