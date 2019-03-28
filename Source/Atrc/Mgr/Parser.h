#pragma once

#include <Atrc/Core/Core/Transform.h>
#include <Atrc/Mgr/Common.h>

namespace Atrc::Mgr::Parser
{

namespace TFloat
{
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    AGZ::Math::Vec3<T> ParseSpectrum(const ConfigNode &node);

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    AGZ::Math::Vec2<T> ParseVec2(const ConfigNode &node);

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    AGZ::Math::Vec3<T> ParseVec3(const ConfigNode &node);

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    AGZ::Math::Rad<T> ParseAngle(const ConfigNode &node);

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    AGZ::Math::RM_Mat3<T> ParseRotateMat(const ConfigNode &node);
}

/*
    (0.1)           => (0.1, 0.1, 0.1)
    (0.1, 0.2, 0.2) => (0.1, 0.2, 0.2)
    b(1)            => (1/255, 1/255, 1/255)
    b(1, 2, 3)      => (1/255, 2/255, 3/255)
*/
inline Spectrum ParseSpectrum(const ConfigNode &node) { return TFloat::ParseSpectrum<Real>(node); }

/*
    (1)    => (1, 1)
    (1, 2) => (1, 2, 3)
*/
Vec2i ParseVec2i(const ConfigNode &node);

/*
    (1)    => (1.0, 1.0)
    (1, 2) => (1.0, 2.0, 3.0)
*/
inline Vec2 ParseVec2(const ConfigNode &node) { return TFloat::ParseVec2<Real>(node); }

/*
    (1)       => (1, 1, 1)
    (1, 2, 3) => (1, 2, 3)
*/
inline Vec3 ParseVec3(const ConfigNode &node) { return TFloat::ParseVec3<Real>(node); }

/*
    Deg(60)  => Rad(Deg(60))
    Rad(1.4) => Rad(1.4)
*/
inline Rad ParseAngle(const ConfigNode &node) { return TFloat::ParseAngle<Real>(node); }

/*
    (t0, t1, t2, ..., tn) => t0 * t1 * t2 * ... * tn
    ti = Translate(Real, Real, Real)
       | Rotate(Vec3, Angle)
       | RotateX(Angle)
       | RotateY(Angle)
       | RotateZ(Angle)
       | Scale(Real)
*/
Transform ParseTransform(const ConfigNode &node);

/*
    (r0, r1, r2, ..., rn) => r0 * r1 * r2 * ... * rn
    ri = Rotate(Vec3, Angle)
       | RotateX(Angle)
       | RotateY(Angle)
       | RotateZ(Angle)
*/
inline Mat3 ParseRotateMat(const ConfigNode &node) { return TFloat::ParseRotateMat<Real>(node); }

/*
    True  => true
    False => false
*/
bool ParseBool(const ConfigNode &node);

namespace TFloat
{
    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
    AGZ::Math::Vec3<T> ParseSpectrum(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY
        {
            if(auto arr = node.TryAsArray())
            {
                if(arr->GetTag() == "b")
                {
                    if(arr->Size() == 1)
                        return AGZ::Math::Vec3<T>((*arr)[0].Parse<T>() / 255);
                    if(arr->Size() == 3)
                        return AGZ::Math::Vec3<T>(
                            (*arr)[0].Parse<T>() / 255,
                            (*arr)[1].Parse<T>() / 255,
                            (*arr)[2].Parse<T>() / 255);
                }
                else if(arr->GetTag().empty())
                {
                    if(arr->Size() == 1)
                        return AGZ::Math::Vec3<T>((*arr)[0].Parse<T>());
                    if(arr->Size() == 3)
                        return AGZ::Math::Vec3<T>(
                            (*arr)[0].Parse<T>(),
                            (*arr)[1].Parse<T>(),
                            (*arr)[2].Parse<T>());
                }
            }

            throw AGZ::HierarchyException("Invalid spectrum form");
        }
        AGZ_HIERARCHY_WRAP("In parsing spectrum: " + node.ToString())
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
    AGZ::Math::Vec2<T> ParseVec2(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY
        {
            if(auto arr = node.TryAsArray())
            {
                if(arr->Size() == 1)
                    return AGZ::Math::Vec2<T>((*arr)[0].Parse<T>());
                if(arr->Size() == 2)
                    return AGZ::Math::Vec2<T>(
                        (*arr)[0].Parse<T>(),
                        (*arr)[1].Parse<T>());
            }
            throw AGZ::HierarchyException("Invalid vec2 form");
        }
        AGZ_HIERARCHY_WRAP("In parsing vec2: " + node.ToString())
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
    AGZ::Math::Vec3<T> ParseVec3(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY
        {
            if(auto arr = node.TryAsArray())
            {
                if(arr->Size() == 1)
                    return AGZ::Math::Vec3<T>((*arr)[0].Parse<T>());
                if(arr->Size() == 3)
                    return AGZ::Math::Vec3<T>(
                        (*arr)[0].Parse<T>(),
                        (*arr)[1].Parse<T>(),
                        (*arr)[2].Parse<T>());
            }
            throw AGZ::HierarchyException("Invalid vec3 form");
        }
        AGZ_HIERARCHY_WRAP("In parsing vec3: " + node.ToString())
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
    AGZ::Math::Rad<T> ParseAngle(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY
        {
            if(auto arr = node.TryAsArray())
            {
                if(arr->Size() != 1)
                    throw AGZ::HierarchyException("Array is too long");
                if(arr->GetTag() == "Deg")
                    return AGZ::Math::Rad<T>(AGZ::Math::Deg<T>((*arr)[0].Parse<T>()));
                if(arr->GetTag() == "Rad")
                    return AGZ::Math::Rad<T>((*arr)[0].Parse<T>());
            }
            throw AGZ::HierarchyException("Invalid angle form");
        }
        AGZ_HIERARCHY_WRAP("In parsing angle: " + node.ToString())
    }

    template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int>>
    AGZ::Math::RM_Mat3<T> ParseRotateMat(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY
        {
            if(!node.IsArray())
                throw AGZ::HierarchyException("Array expected");
            auto &arr = node.AsArray();

            auto ret = AGZ::Math::RM_Mat3<T>::IDENTITY();
            for(size_t i = 0; i < arr.Size(); ++i)
            {
                if(!arr[i].IsArray())
                    throw AGZ::HierarchyException("Array expected");
                auto &unit = arr[i].AsArray();

                AGZ_HIERARCHY_TRY
                {
                    if(unit.GetTag() == "Rotate")
                    {
                        if(unit.Size() != 2)
                            throw AGZ::HierarchyException("Rotate size must be 2");
                        ret = ret * AGZ::Math::RM_Mat3<T>::template Rotate(
                            ParseVec3<T>(unit[0]), ParseAngle<T>(unit[1]));
                    }
                    else if(unit.GetTag() == "RotateX")
                    {
                        if(unit.Size() != 1)
                            throw AGZ::HierarchyException("RotateX size must be 1");
                        ret = ret * AGZ::Math::RM_Mat3<T>::template RotateX(ParseAngle<T>(unit[0]));
                    }
                    else if(unit.GetTag() == "RotateY")
                    {
                        if(unit.Size() != 1)
                            throw AGZ::HierarchyException("RotateY size must be 1");
                        ret = ret * AGZ::Math::RM_Mat3<T>::template RotateY(ParseAngle<T>(unit[0]));
                    }
                    else if(unit.GetTag() == "RotateZ")
                    {
                        if(unit.Size() != 1)
                            throw AGZ::HierarchyException("RotateZ size must be 1");
                        ret = ret * AGZ::Math::RM_Mat3<T>::template RotateZ(ParseAngle<T>(unit[0]));
                    }
                    else
                        throw AGZ::HierarchyException("Unknown rotation type");
                }
                AGZ_HIERARCHY_WRAP("In parsing rotation unit: " + unit.ToString());
            }

            return ret;
        }
        AGZ_HIERARCHY_WRAP("In parsing rotation matrix: " + node.ToString())
    }
}

} // namespace Atrc::Mgr::Parser
