#pragma once

#include <memory>
#include <optional>

#include <Utils.h>

AGZ_NS_BEG(Atrc)

// ============================= Option =============================

template<typename T>
using Option = std::optional<T>;

template<typename T>
auto Some(T &&value)
{
    return std::make_optional<T>(std::forward<T>(value));
}

inline std::nullopt_t None = std::nullopt;

// ============================= RefCounted Object =============================

template<typename T>
using RC = std::shared_ptr<T>;

template<typename T, typename...Args>
auto NewRC(Args&&...args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// ============================= Atrc::Exception =============================

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

// ============================= Clearer OO-related 'keyword' =============================

#define ATRC_INTERFACE  class
#define ATRC_IMPLEMENTS public
#define ATRC_PROPERTY   public

// ============================= Export AGZ::Math =============================

using namespace AGZ::Math;

using Real = double;
using RealT = FP<Real>;

using Vec2r = Vec2<Real>;
using Vec3r = Vec3<Real>;
using Vec4r = Vec4<Real>;

using Mat3r = Mat3<Real>;
using Mat4r = Mat4<Real>;

using Radr = Rad<Real>;
using Degr = Deg<Real>;

AGZ_NS_END(Atrc)
