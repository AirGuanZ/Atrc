#include "FileBrowser.h"

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

    for(auto &u : curUnits_)
    {
        bool selected = u.name == selectedName_;
        std::string displayName = (u.isDir ? "[d] " : "[f] ") + u.name;
        if(ImGui::Selectable(displayName.c_str(), selected, ImGuiSelectableFlags_DontClosePopups))
            selectedName_ = u.name;
    }

    if(ImGui::Button("cancel"))
    {
        ImGui::CloseCurrentPopup();
        return false;
    }

    // TODO

    return false;
}

const std::string& FileBrowser::GetLabel() const noexcept
{
    return label_;
}

std::string FileBrowser::GetResult() const
{
    return (pwd_ / selectedName_).string();
}

void FileBrowser::UpdateCurrentUnits()
{
    curUnits_.clear();

    if(!is_directory(pwd_))
        return;

    std::vector<std::string> names;
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
