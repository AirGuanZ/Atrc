#pragma once

#include <optional>
#include <Utils.h>

AGZ_NS_BEG(Atrc)

template<typename T>
using Option = std::optional<T>;

template<typename T, typename...Args>
auto MakeSome(Args&&...args)
{
    return std::make_optional<T>(
                std::forward<Args>(args)...);
}

template<typename T>
auto Some(T &&arg)
{
    return std::optional<AGZ::remove_rcv_t<T>>(
                    std::forward<T>(arg));
}

inline auto None = std::nullopt;

AGZ_NS_END(Atrc)

#define ATRC_INTERFACE class
#define ATRC_IMPLEMENTS public
#define ATRC_PROPERTY public
