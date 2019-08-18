#pragma once

#include <agz/tracer_utility/math.h>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <sol/sol.hpp>

AGZ_TRACER_BEGIN

namespace lua
{

inline sol::table to_table(sol::state &state, const ConfigGroup &group);

/**
 * @brief 将ConfigArray转为lua table
 */
inline sol::table to_table(sol::state &state, const ConfigArray &arr)
{
    auto ret = state.create_table();
    for(size_t i = 0; i < arr.size(); ++i)
    {
        auto &right = arr.at(i);
        if(right.is_group())
            ret[i] = to_table(state, right.as_group());
        else if(right.is_array())
            ret[i] = to_table(state, right.as_array());
        else
            ret[i] = right.as_value().as_str();
    }
    return ret;
}

/**
 * @brief 将ConfigGroup转为lua table
 */
inline sol::table to_table(sol::state &state, const ConfigGroup &group)
{
    auto ret = state.create_table();
    for(auto &p : group)
    {
        if(p.second->is_group())
            ret[p.first] = to_table(state, p.second->as_group());
        else if(p.second->is_array())
            ret[p.first] = to_table(state, p.second->as_array());
        else
            ret[p.first] = p.second->as_value().as_str();
    }
    return ret;
}

/** @brief table{ x, y, z } -> Vec3 */
inline Vec3 table_to_vec3(const sol::table &tab)
{
    real x = tab.get<real>("x");
    real y = tab.get<real>("y");
    real z = tab.get<real>("z");
    return { x, y, z };
}

/** @brief table{ r, g, b } -> Spectrum */
inline Spectrum table_to_spectrum(const sol::table &tab)
{
    real r = tab.get<real>("r");
    real g = tab.get<real>("g");
    real b = tab.get<real>("b");
    return { r, g, b };
}

/**
 * @brief 在指定lua state中创建一个与tab具有相同内容的table
 * 
 * key、value类型只允许为：
 *  string, number, boolean, table
 */
inline sol::table clone_table(const sol::table &tab, sol::state &cloner)
{
    sol::table ret = cloner.create_table();

    auto set_right = [&](auto left, const sol::object &right)
    {
        switch(right.get_type())
        {
        case sol::type::string:  left = right.as<std::string>();                     break;
        case sol::type::number:  left = right.as<real>();                            break;
        case sol::type::boolean: left = right.as<bool>();                            break;
        case sol::type::table:   left = clone_table(right.as<sol::table>(), cloner); break;
        default:
            left = 0;
        }
    };

    tab.for_each([&](const std::pair<sol::object, sol::object> &pair)
    {
        auto &left = pair.first;
        auto ltype = left.get_type();
        switch(ltype)
        {
        case sol::type::string:  set_right(ret[left.as<std::string>()], pair.second);  break;
        case sol::type::number:  set_right(ret[left.as<real>()],        pair.second);  break;
        case sol::type::boolean: set_right(ret[left.as<bool>()],        pair.second);  break;
        case sol::type::table:   set_right(ret[left.as<sol::table>()],   pair.second); break;
        default:
            break;
        }
    });

    return ret;
}

} // namespace lua

AGZ_TRACER_END
