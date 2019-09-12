#include <string>

#include <agz/common/config.h>
#include <agz/utility/misc.h>
#include <agz/utility/string.h>

AGZ_TRACER_BEGIN

namespace
{
    Vec2 to_vec2(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        auto &arr = node.as_array();

        if(arr.size() == 1)
        {
            real val = arr.at_real(0);
            return Vec2(val);
        }

        if(arr.size() == 2)
        {
            real x = arr.at_real(0);
            real y = arr.at_real(1);
            return Vec2(x, y);
        }

        throw ConfigException(stdstr::cat("invalid array size (actual = ", arr.size(), ")"));

        AGZ_HIERARCHY_WRAP("in config to_vec2")
    }

    Vec3 to_vec3(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        auto &arr = node.as_array();

        if(arr.size() == 1)
        {
            real val = arr.at_real(0);
            return Vec3(val);
        }

        if(arr.size() == 3)
        {
            real x = arr.at_real(0);
            real y = arr.at_real(1);
            real z = arr.at_real(2);
            return Vec3(x, y, z);
        }

        throw ConfigException(stdstr::cat("invalid array size (actual = ", arr.size(), ")"));

        AGZ_HIERARCHY_WRAP("in config to_vec3")
    }

    Vec4 to_vec4(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        auto &arr = node.as_array();

        if(arr.size() == 1)
        {
            real val = arr.at_real(0);
            return Vec4(val);
        }

        if(arr.size() == 4)
        {
            real x = arr.at_real(0);
            real y = arr.at_real(1);
            real z = arr.at_real(2);
            real w = arr.at_real(3);
            return Vec4(x, y, z, w);
        }

        throw ConfigException(stdstr::cat("invalid array size (actual = ", arr.size(), ")"));

        AGZ_HIERARCHY_WRAP("in config to_vec4")
    }

    Transform2 to_basic_transform2(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        auto &group = node.as_group();
        auto &type = group.child_str("type");

        if(stdstr::ends_with(type, "//"))
            return Transform2();

        if(type == "translate")
        {
            Vec2 offset = group.child_vec2("offset");
            return Transform2::translate(offset);
        }

        if(type == "rotate")
        {
            real rad;
            if(auto p_node = group.find_child("rad"))
                rad = p_node->as_value().as_real();
            else
                rad = group.child_real("deg") / 180 * PI_r;
            return Transform2::rotate(rad);
        }

        if(type == "scale")
        {
            real ratio = group.child_real("ratio");
            return Transform2::scale(ratio, ratio);
        }

        throw ConfigException(stdstr::cat("unknown transform2 type (type = ", type, ")"));

        AGZ_HIERARCHY_WRAP("in config to_basic_transform2")
    }

    Transform2 to_transform2(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        if(node.is_group())
            return to_basic_transform2(node);

        auto &arr = node.as_array();

        Transform2 ret;
        for(size_t i = 0; i < arr.size(); ++i)
            ret *= arr.at_transform2(i);
        return ret;

        AGZ_HIERARCHY_WRAP("in config to_transform2")
    }

    Transform3 to_basic_transform3(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        auto &group = node.as_group();
        auto &type = group.child_str("type");

        if(stdstr::ends_with(type, "//"))
            return Transform3();

        if(type == "translate")
        {
            Vec3 offset = group.child_vec3("offset");
            return Transform3::translate(offset);
        }

        if(type == "rotate")
        {
            Vec3 axis = group.child_vec3("axis");
            real rad;
            if(auto p_node = group.find_child("rad"))
                rad = p_node->as_value().as_real();
            else
                rad = group.child_real("deg") / 180 * PI_r;
            return Transform3::rotate(axis, rad);
        }

        if(type == "rotate_x")
        {
            real rad;
            if(auto p_node = group.find_child("rad"))
                rad = p_node->as_value().as_real();
            else
                rad = group.child_real("deg") / 180 * PI_r;
            return Transform3::rotate_x(rad);
        }

        if(type == "rotate_y")
        {
            real rad;
            if(auto p_node = group.find_child("rad"))
                rad = p_node->as_value().as_real();
            else
                rad = group.child_real("deg") / 180 * PI_r;
            return Transform3::rotate_y(rad);
        }

        if(type == "rotate_z")
        {
            real rad;
            if(auto p_node = group.find_child("rad"))
                rad = p_node->as_value().as_real();
            else
                rad = group.child_real("deg") / 180 * PI_r;
            return Transform3::rotate_z(rad);
        }

        if(type == "scale")
        {
            real ratio = group.child_real("ratio");
            return Transform3::scale(ratio, ratio, ratio);
        }

        throw ConfigException(stdstr::cat("unknown transform3 type (type = ", type, ")"));

        AGZ_HIERARCHY_WRAP("in config to_basic_transform3")
    }

    Transform3 to_transform3(const ConfigNode &node)
    {
        AGZ_HIERARCHY_TRY

        if(node.is_group())
            return to_basic_transform3(node);

        auto &arr = node.as_array();

        Transform3 ret;
        for(size_t i = 0; i < arr.size(); ++i)
            ret *= arr.at_transform3(i);
        return ret;

        AGZ_HIERARCHY_WRAP("in config to_transform3")
    }
}

bool ConfigNode::is_group() const noexcept
{
    return false;
}

bool ConfigNode::is_array() const noexcept
{
    return false;
}

bool ConfigNode::is_value() const noexcept
{
    return false;
}

ConfigGroup &ConfigNode::as_group()
{
    throw ConfigException("failed to convert ConfigNode to ConfigGroup (actual = " + std::string(this->type()) + ")");
}

ConfigArray &ConfigNode::as_array()
{
    throw ConfigException("failed to convert ConfigNode to ConfigArray (actual = " + std::string(this->type()) + ")");
}

ConfigValue &ConfigNode::as_value()
{
    throw ConfigException("failed to convert ConfigNode to ConfigValue (actual = " + std::string(this->type()) + ")");
}

const ConfigGroup &ConfigNode::as_group() const
{
    throw ConfigException("failed to convert ConfigNode to ConfigGroup (actual = " + std::string(this->type()) + ")");
}

const ConfigArray &ConfigNode::as_array() const
{
    throw ConfigException("failed to convert ConfigNode to ConfigArray (actual = " + std::string(this->type()) + ")");
}

const ConfigValue &ConfigNode::as_value() const
{
    throw ConfigException("failed to convert ConfigNode to ConfigValue (actual = " + std::string(this->type()) + ")");
}

const char *ConfigGroup::type() const noexcept
{
    return "ConfigGroup";
}

bool ConfigGroup::is_group() const noexcept
{
    return true;
}

ConfigGroup &ConfigGroup::as_group()
{
    return *this;
}

const ConfigGroup &ConfigGroup::as_group() const
{
    return *this;
}

const ConfigGroup *ConfigGroup::find_child_group(const std::string &name) const
{
    if(auto node = find_child(name))
        return &node->as_group();
    return nullptr;
}

const ConfigArray *ConfigGroup::find_child_array(const std::string &name) const
{
    if(auto node = find_child(name))
        return &node->as_array();
    return nullptr;
}

const ConfigValue *ConfigGroup::find_child_value(const std::string &name) const
{
    if(auto node = find_child(name))
        return &node->as_value();
    return nullptr;
}

const ConfigNode *ConfigGroup::find_child(const std::string &name) const
{
    auto it = group_.find(name);
    return it != group_.end() ? it->second.get() : nullptr;
}

ConfigNode &ConfigGroup::child(const std::string &name)
{
    auto it = group_.find(name);
    if(it == group_.end())
        throw ConfigException(stdstr::cat("key not found in ConfigGroup (key = ", name, ")"));
    return *it->second;
}

ConfigGroup &ConfigGroup::child_group(const std::string &name)
{
    return child(name).as_group();
}

ConfigArray &ConfigGroup::child_array(const std::string &name)
{
    return child(name).as_array();
}

ConfigValue &ConfigGroup::child_value(const std::string &name)
{
    return child(name).as_value();
}

const ConfigNode &ConfigGroup::child(const std::string &name) const
{
    auto it = group_.find(name);
    if(it == group_.end())
        throw ConfigException(stdstr::cat("key not found in ConfigGroup (key = ", name, ")"));
    return *it->second;
}

const ConfigGroup &ConfigGroup::child_group(const std::string &name) const
{
    return child(name).as_group();
}

const ConfigArray &ConfigGroup::child_array(const std::string &name) const
{
    return child(name).as_array();
}

const ConfigValue &ConfigGroup::child_value(const std::string &name) const
{
    return child(name).as_value();
}

Vec2 ConfigGroup::child_vec2(const std::string &name) const
{
    return to_vec2(child(name));
}

Vec3 ConfigGroup::child_vec3(const std::string &name) const
{
    return to_vec3(child(name));
}

Vec4 ConfigGroup::child_vec4(const std::string &name) const
{
    return to_vec4(child(name));
}

Spectrum ConfigGroup::child_spectrum(const std::string &name) const
{
    return Spectrum(to_vec3(child(name)));
}

Transform2 ConfigGroup::child_transform2(const std::string &name) const
{
    return to_transform2(child(name));
}

Transform3 ConfigGroup::child_transform3(const std::string &name) const
{
    return to_transform3(child(name));
}

int ConfigGroup::child_int(const std::string &name) const
{
    return child_value(name).as_int();
}

float ConfigGroup::child_float(const std::string &name) const
{
    return child_value(name).as_float();
}

double ConfigGroup::child_double(const std::string &name) const
{
    return child_value(name).as_double();
}

real ConfigGroup::child_real(const std::string &name) const
{
    return child_value(name).as_real();
}

int ConfigGroup::child_int_or(const std::string &name, int default_val) const
{
    if(auto node = find_child(name))
        return node->as_value().as_int();
    return default_val;
}

real ConfigGroup::child_real_or(const std::string &name, real default_val) const
{
    if(auto node = find_child(name))
        return node->as_value().as_real();
    return default_val;
}

const std::string &ConfigGroup::child_str(const std::string &name) const
{
    return child_value(name).as_str();
}

std::string ConfigGroup::child_str_or(const std::string &name, const std::string &default_val) const
{
    if(auto node = find_child(name))
        return node->as_value().as_str();
    return default_val;
}

void ConfigGroup::insert_child(const std::string &name, std::shared_ptr<ConfigNode> child)
{
    assert(child != nullptr);
    group_[name] = std::move(child);
}

const char *ConfigArray::type() const noexcept
{
    return "ConfigArray";
}

bool ConfigArray::is_array() const noexcept
{
    return true;
}

ConfigArray &ConfigArray::as_array()
{
    return *this;
}

const ConfigArray& ConfigArray::as_array() const
{
    return *this;
}

size_t ConfigArray::size() const noexcept
{
    return array_.size();
}

ConfigNode &ConfigArray::at(size_t idx)
{
    if(idx >= size())
        throw ConfigException(stdstr::cat("config array: index out of range. (size = ", size(), ", idx = ", idx, ")"));
    return *array_[idx];
}

ConfigGroup &ConfigArray::at_group(size_t idx)
{
    return at(idx).as_group();
}

ConfigArray &ConfigArray::at_array(size_t idx)
{
    return at(idx).as_array();
}

ConfigValue &ConfigArray::at_value(size_t idx)
{
    return at(idx).as_value();
}

const ConfigNode &ConfigArray::at(size_t idx) const
{
    if(idx >= size())
        throw ConfigException(stdstr::cat("config array: index out of range. (size = ", size(), ", idx = ", idx, ")"));
    return *array_[idx];
}

const ConfigGroup &ConfigArray::at_group(size_t idx) const
{
    return at(idx).as_group();
}

const ConfigArray &ConfigArray::at_array(size_t idx) const
{
    return at(idx).as_array();
}

const ConfigValue &ConfigArray::at_value(size_t idx) const 
{
    return at(idx).as_value();
}

Vec2 ConfigArray::at_vec2(size_t idx) const
{
    return to_vec2(at(idx));
}

Vec3 ConfigArray::at_vec3(size_t idx) const
{
    return to_vec3(at(idx));
}

Vec4 ConfigArray::at_vec4(size_t idx) const
{
    return to_vec4(at(idx));
}

Spectrum ConfigArray::at_spectrum(size_t idx) const
{
    return Spectrum(at_vec3(idx));
}

Transform2 ConfigArray::at_transform2(size_t idx) const
{
    return to_transform2(at(idx));
}

Transform3 ConfigArray::at_transform3(size_t idx) const
{
    return to_transform3(at(idx));
}

int ConfigArray::at_int(size_t idx) const
{
    return at_value(idx).as_int();
}

float ConfigArray::at_float(size_t idx) const
{
    return at_value(idx).as_float();
}

double ConfigArray::at_double(size_t idx) const
{
    return at_value(idx).as_double();
}

real ConfigArray::at_real(size_t idx) const
{
    return at_value(idx).as_real();
}

const std::string& ConfigArray::at_str(size_t idx) const
{
    return at_value(idx).as_str();
}

void ConfigArray::push_back(std::shared_ptr<ConfigNode> elem)
{
    array_.push_back(std::move(elem));
}

ConfigValue::ConfigValue(std::string value)
    : value_(std::move(value))
{
    
}

const char *ConfigValue::type() const noexcept
{
    return "ConfigValye";
}

bool ConfigValue::is_value() const noexcept
{
    return true;
}

ConfigValue &ConfigValue::as_value()
{
    return *this;
}

const ConfigValue &ConfigValue::as_value() const
{
    return *this;
}

int ConfigValue::as_int() const
{
    return std::stoi(value_);
}

float ConfigValue::as_float() const
{
    return std::stof(value_);
}

double ConfigValue::as_double() const
{
    return std::stod(value_);
}

real ConfigValue::as_real() const
{
    return stdstr::from_string<real>(value_);
}

const std::string &ConfigValue::as_str() const
{
    return value_;
}

void ConfigValue::set(std::string value)
{
    value_ = std::move(value);
}

AGZ_TRACER_END
