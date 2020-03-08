#pragma once

#include <filesystem>
#include <unordered_map>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class JSONExportContext
{
    std::filesystem::path scene_asset_dir_;
    std::string ref_filename_prefix_;

    std::unordered_map<std::string, int> ext2idx_;

public:

    explicit JSONExportContext(const std::string &scene_desc_filename);

    // returns (refname, filename)
    std::pair<std::string, std::string> gen_filename(const std::string &ext);
};

AGZ_EDITOR_END
