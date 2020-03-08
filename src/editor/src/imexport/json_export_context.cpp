#include <filesystem>

#include <agz/editor/imexport/json_export_context.h>

AGZ_EDITOR_BEGIN

JSONExportContext::JSONExportContext(const std::string &scene_desc_filename)
{
    const auto p = absolute(std::filesystem::path(scene_desc_filename));
    scene_asset_dir_ = p.parent_path() / (p.filename().string() + ".asset");
    ref_filename_prefix_ = "${scene-directory}/" + p.filename().string() + ".asset/";

    create_directories(scene_asset_dir_);
}

std::pair<std::string, std::string> JSONExportContext::gen_filename(const std::string &ext)
{
    const std::string name = std::to_string(ext2idx_[ext]++) + ext;
    return { ref_filename_prefix_ + name, (scene_asset_dir_ / name).string() };
}

AGZ_EDITOR_END
