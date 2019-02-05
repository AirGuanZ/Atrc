#include <Atrc/ModelViewer/FileBrowser.h>

void FileBrowser::SetLabel(std::string label)
{
    label_ = std::move(label);
}

void FileBrowser::SetTarget(bool selectDirectory)
{
    selectDirectory_ = selectDirectory;
}

void FileBrowser::SetCurrentDirectory(std::string_view dir)
{
    selectedName_ = std::string();
    pwd_ = dir;
    if(!is_directory(pwd_))
        pwd_ = AGZ::FileSys::File::GetWorkingDirectory();
    AGZ_ASSERT(is_directory(pwd_));
    UpdateCurrentUnits();
}

bool FileBrowser::Display()
{
    if(!ImGui::BeginPopupModal(label_.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return false;
    AGZ::ScopeGuard popupGuard([] { ImGui::EndPopup(); });

    const std::string *enterDir = nullptr;

    auto pwdStr = INV_WIDEN(pwd_.wstring());
    ImGui::Text("%s", pwdStr.c_str());

    ImGui::BeginChild("files", ImVec2(640, 300));
    for(auto &u : curUnits_)
    {
        bool selected = u.name == selectedName_;
        std::string displayName = (u.isDir ? "[d] " : "[f] ") + u.name;
        if(ImGui::Selectable(displayName.c_str(), selected, ImGuiSelectableFlags_DontClosePopups) && (u.isDir == selectDirectory_))
            selectedName_ = u.name;
        if(ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0) && u.isDir)
            enterDir = &u.name;
    }
    ImGui::EndChild();

    if(enterDir)
    {
        if(*enterDir == "..")
            SetCurrentDirectory(pwd_.parent_path().string());
        else
            SetCurrentDirectory((pwd_ / *enterDir).string());
    }

    if(ImGui::Button("ok") && !selectedName_.empty())
    {
        ImGui::CloseCurrentPopup();
        return true;
    }

    ImGui::SameLine();

    if(ImGui::Button("cancel"))
    {
        ImGui::CloseCurrentPopup();
        return false;
    }

    return false;
}

const std::string& FileBrowser::GetLabel() const noexcept
{
    return label_;
}

std::string FileBrowser::GetResult(bool relative) const
{
    auto p = pwd_ / selectedName_;
    return (relative ?  std::filesystem::relative(p) : absolute(p)).string();
}

void FileBrowser::UpdateCurrentUnits()
{
    curUnits_.clear();

    if(!is_directory(pwd_))
        return;

    curUnits_.push_back({ true, ".." });

    for(auto &p : std::filesystem::directory_iterator(pwd_))
    {
        Unit unit;
#ifdef AGZ_OS_WIN32
        unit.name = INV_WIDEN(p.path().filename().wstring());
#else
        unit.name = p.path().filename().string();
#endif
        if(p.is_regular_file())
            unit.isDir = false;
        else if(p.is_directory())
            unit.isDir = true;
        else
            continue;

        if(AGZ::StartsWith(unit.name, "$"))
            continue;

        curUnits_.push_back(std::move(unit));
    }

    std::sort(begin(curUnits_), end(curUnits_), 
        [](auto &L, auto &R)
    {
        if(L.isDir ^ R.isDir)
            return L.isDir;
        return L.name < R.name;
    });
}
