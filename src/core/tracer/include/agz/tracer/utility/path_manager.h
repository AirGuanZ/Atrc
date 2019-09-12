#pragma once

#include <filesystem>
#include <map>

#include <agz/common/common.h>
#include <agz/utility/string.h>

AGZ_TRACER_BEGIN
    
/**
 * @brief 路径替换辅助设施
 */
class PathManager
{
    std::map<std::string, std::string> replacers_;

public:

    /**
     * @brief 添加一个路径替换
     */
    void add_replacer(const std::string &key, const std::string &value)
    {
        replacers_[key] = value;
    }

    /**
     * @brief 对一个路径实施替换
     */
    std::string get(const std::string &s) const
    {
        std::string ret(s);
        for(auto &p : replacers_)
            stdstr::replace_(ret, p.first, p.second);
        return absolute(std::filesystem::path(ret)).lexically_normal().string();
    }
};

#define WORKING_DIR_PATH_NAME "${working-directory}"
#define SCENE_DESC_PATH_NAME  "${scene-directory}"

AGZ_TRACER_END
