#pragma once

#include <Atrc/Editor/GL.h>
#include <Lib/ImFileBrowser/imfilebrowser.h>

class FileSelector
{
    ImGui::FileBrowser fileBrowser_;
    std::filesystem::path filename_;
    std::string u8str_;

public:

    explicit FileSelector(ImGuiFileBrowserFlags flags = 0)
        : fileBrowser_(flags)
    {

    }

    bool Display()
    {
        ImGui::PushID(this);
        AGZ::ScopeGuard popID([] { ImGui::PopID(); });
        
        if(ImGui::Button("browse"))
            fileBrowser_.Open();
        fileBrowser_.Display();

        bool ret = false;
        if(fileBrowser_.HasSelected())
        {
            SetFilename(fileBrowser_.GetSelected());
            fileBrowser_.ClearSelected();
            ret = true;
        }

        ImGui::SameLine();

        ImGui::Text("%s", u8str_.c_str());

        return ret;
    }

    void SetFilename(const std::filesystem::path &path)
    {
        filename_ = relative(path);
        u8str_ = filename_.u8string();
    }

    void Clear()
    {
        SetFilename("");
    }

    bool HasFilename() const noexcept
    {
        return !filename_.empty();
    }

    const std::filesystem::path &GetFilename() const noexcept
    {
        return filename_;
    }

    std::filesystem::path RelativeTo(const std::filesystem::path &dir) const
    {
        return relative(filename_, dir);
    }
};
