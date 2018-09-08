#pragma once

#include <optional>
#include <Utils.h>

AGZ_NS_BEG(Atrc)

template<typename T>
using Option = std::optional<T>;

template<typename T, typename...Args>
auto Some(Args&&...args)
{
    return std::make_optional<T>(
                std::forward<Args>(args)...);
}

inline auto None = std::nullopt;

AGZ_NS_END(Atrc)
