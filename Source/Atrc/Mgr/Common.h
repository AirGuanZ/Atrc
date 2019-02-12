#pragma once

#include <filesystem>

#include <AGZUtils/Utils/Config.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc::Mgr
{

using AGZ::Config;
using AGZ::ConfigArray;
using AGZ::ConfigGroup;
using AGZ::ConfigNode;
using AGZ::ConfigValue;

class MgrErr : public std::runtime_error
{
    std::shared_ptr<std::exception> leaf_;
    std::shared_ptr<MgrErr> interior_;

public:

    MgrErr(const MgrErr &interior, const std::string &msg)
        : runtime_error(msg), interior_(std::make_shared<MgrErr>(interior)) { }

    MgrErr(const std::exception &leaf, const std::string &msg)
        : runtime_error(msg), leaf_(std::make_shared<std::runtime_error>(leaf.what())) { }

    explicit MgrErr(const std::string &msg) : runtime_error(msg) { }

    const char *GetMsg() const noexcept { return what(); }

    const MgrErr *TryGetInterior() const noexcept { return interior_.get(); }

    const std::exception *TryGetLeaf() const noexcept { return leaf_.get(); }
};

#define ATRC_MGR_TRY try

#define ATRC_MGR_CATCH_AND_RETHROW(MSG) \
    catch(const ::Atrc::Mgr::MgrErr &err) \
    { \
        throw ::Atrc::Mgr::MgrErr(err, (MSG)); \
    } \
    catch(const ::std::exception &err) \
    { \
        throw ::Atrc::Mgr::MgrErr(err, (MSG)); \
    } \
    catch(...) \
    { \
        throw ::Atrc::Mgr::MgrErr( \
            ::Atrc::Mgr::MgrErr("An unknown error occurred"), (MSG)); \
    }

inline std::string GetCacheFilename(std::string_view filename)
{
    std::filesystem::path parent("./.agz.cache/");
    parent.append(filename);
    return parent.relative_path().string();
}

} // namespace Atrc::Mgr
