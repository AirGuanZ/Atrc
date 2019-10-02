#pragma once

#include <map>
#include <stdexcept>
#include <string>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

class ConfigException : public std::runtime_error
{
public:

    using std::runtime_error::runtime_error;
};

class ConfigNode;
class ConfigGroup;
class ConfigArray;
class ConfigValue;

class ConfigNode
{
public:

    virtual ~ConfigNode() = default;

    virtual const char *type() const noexcept = 0;

    virtual bool is_group() const noexcept;
    virtual bool is_array() const noexcept;
    virtual bool is_value() const noexcept;

    virtual ConfigGroup &as_group();
    virtual ConfigArray &as_array();
    virtual ConfigValue &as_value();

    virtual const ConfigGroup &as_group() const;
    virtual const ConfigArray &as_array() const;
    virtual const ConfigValue &as_value() const;
};

class ConfigGroup : public ConfigNode
{
    std::map<std::string, std::shared_ptr<ConfigNode>> group_;

public:

    const char *type() const noexcept override;

    bool is_group() const noexcept override;

          ConfigGroup& as_group()       override;
    const ConfigGroup& as_group() const override;

    const ConfigNode  *find_child(const std::string &name) const;
    const ConfigGroup *find_child_group(const std::string &name) const;
    const ConfigArray *find_child_array(const std::string &name) const;
    const ConfigValue *find_child_value(const std::string &name) const;

    ConfigNode  &child      (const std::string &name);
    ConfigGroup &child_group(const std::string &name);
    ConfigArray &child_array(const std::string &name);
    ConfigValue &child_value(const std::string &name);

    const ConfigNode  &child      (const std::string &name) const;
    const ConfigGroup &child_group(const std::string &name) const;
    const ConfigArray &child_array(const std::string &name) const;
    const ConfigValue &child_value(const std::string &name) const;

    Vec2 child_vec2(const std::string &name) const;
    Vec3 child_vec3(const std::string &name) const;
    Vec4 child_vec4(const std::string &name) const;

    Vec2 child_vec2_or(const std::string &name, const Vec2 &default_val) const;
    Vec3 child_vec3_or(const std::string &name, const Vec3 &default_val) const;

    Spectrum   child_spectrum   (const std::string &name) const;
    Spectrum   child_spectrum_or(const std::string &name, const Spectrum &default_val) const;

    Transform2 child_transform2(const std::string &name) const;
    Transform3 child_transform3(const std::string &name) const;

    int    child_int   (const std::string &name) const;
    float  child_float (const std::string &name) const;
    double child_double(const std::string &name) const;
    real   child_real  (const std::string &name) const;

    int child_int_or  (const std::string &name, int default_val)  const;
    real child_real_or(const std::string &name, real default_val) const;

    const std::string &child_str(const std::string &name) const;
    std::string        child_str_or(const std::string &name, const std::string &default_val) const;

    void insert_child(const std::string &name, std::shared_ptr<ConfigNode> child);

    auto begin() { return group_.begin(); }
    auto end()   { return group_.end();   }
    
    auto begin() const { return group_.begin(); }
    auto end()   const { return group_.end();   }

    size_t size() const noexcept { return group_.size(); }
};

class ConfigArray : public ConfigNode
{
    std::vector<std::shared_ptr<ConfigNode>> array_;

public:

    const char *type() const noexcept override;

    bool is_array() const noexcept override;

          ConfigArray& as_array()       override;
    const ConfigArray& as_array() const override;

    size_t size() const noexcept;
    
    ConfigNode  &at      (size_t idx);
    ConfigGroup &at_group(size_t idx);
    ConfigArray &at_array(size_t idx);
    ConfigValue &at_value(size_t idx);
    
    const ConfigNode  &at      (size_t idx) const;
    const ConfigGroup &at_group(size_t idx) const;
    const ConfigArray &at_array(size_t idx) const;
    const ConfigValue &at_value(size_t idx) const;

    Vec2 at_vec2(size_t idx) const;
    Vec3 at_vec3(size_t idx) const;
    Vec4 at_vec4(size_t idx) const;

    Spectrum   at_spectrum (size_t idx) const;
    Transform2 at_transform2(size_t idx) const;
    Transform3 at_transform3(size_t idx) const;

    int    at_int   (size_t idx) const;
    float  at_float (size_t idx) const;
    double at_double(size_t idx) const;
    real   at_real  (size_t idx) const;

    const std::string &at_str(size_t idx) const;

    void push_back(std::shared_ptr<ConfigNode> elem);
};

class ConfigValue : public ConfigNode
{
    std::string value_;

public:

    explicit ConfigValue(std::string value = "");

    const char *type() const noexcept override;

    bool is_value() const noexcept override;

          ConfigValue& as_value()       override;
    const ConfigValue& as_value() const override;

    int    as_int   () const;
    float  as_float () const;
    double as_double() const;
    real   as_real  () const;

    const std::string &as_str() const;

    void set(std::string value);
};

using Config = ConfigGroup;

AGZ_TRACER_END
