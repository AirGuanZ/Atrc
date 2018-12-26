#pragma once

#include <Utils/Config.h>
#include <Utils/FileSys.h>

#include <Atrc/Lib/Core/Common.h>

namespace Atrc::Mgr
{

using AGZ::Config;
using AGZ::ConfigArray;
using AGZ::ConfigGroup;
using AGZ::ConfigNode;
using AGZ::ConfigValue;
using AGZ::FileSys::Path8;
using AGZ::Str8;

class MgrErr
{
    std::shared_ptr<std::exception> leaf_;
    std::shared_ptr<MgrErr> interior_;
    Str8 msg_;

public:

    MgrErr(const MgrErr &interior, Str8 msg)
        : interior_(std::make_shared<MgrErr>(interior)), msg_(std::move(msg)) { }

    MgrErr(const std::exception &leaf, Str8 msg)
        : leaf_(std::make_shared<std::runtime_error>(leaf.what())), msg_(std::move(msg)) { }

    explicit MgrErr(Str8 msg) : msg_(std::move(msg)) { }

    const Str8 &GetMsg() const noexcept { return msg_; }

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

inline Str8 GetCacheFilename(const Str8 &filename)
{
    return AGZ::FileSys::Path8("./.agz.cache/").Append(
        AGZ::FileSys::Path8(filename).ToRelative()).ToStr();
}

} // namespace Atrc::Mgr
