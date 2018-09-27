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

class Exception : public std::runtime_error
{
public:
    explicit Exception(const std::string &err) : runtime_error(err) { }
};

#define DEFINE_ATRC_EXCEPTION(NAME) \
    class NAME : public ::Atrc::Exception \
    { \
    public: \
        explicit NAME(const std::string &err) : Exception(err) { } \
    }

DEFINE_ATRC_EXCEPTION(ArgumentException);

AGZ_NS_END(Atrc)

#define ATRC_INTERFACE class
#define ATRC_IMPLEMENTS public
#define ATRC_PROPERTY public
