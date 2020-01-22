#include <agz/tracer/utility/config_cvt.h>

using namespace agz;
using namespace tracer;

namespace
{

    JSON to_json_impl(const ConfigNode &node)
    {
        if(node.is_group())
        {
            JSON ret;
            for(auto p : node.as_group())
            {
                if(p.second->is_value())
                    ret[p.first] = p.second->as_value().as_str();
                else
                    ret[p.first] = to_json_impl(*p.second);
            }
            return ret;
        }

        assert(node.is_array());
        JSON ret = JSON::array();
        const auto &arr = node.as_array();
        for(size_t i = 0; i < arr.size(); ++i)
        {
            auto &elem = arr.at(i);
            if(elem.is_value())
                ret.push_back(elem.as_value().as_str());
            else
                ret.push_back(to_json_impl(elem));
        }
        return ret;
    }

    std::shared_ptr<ConfigValue> get_child(const JSON &val)
    {
        if(val.is_boolean())
            return std::make_shared<ConfigValue>(val.get<bool>() ? "1" : "0");
        if(val.is_number_float())
            return std::make_shared<ConfigValue>(std::to_string(val.get<real>()));
        if(val.is_number_integer())
            return std::make_shared<ConfigValue>(std::to_string(val.get<int>()));
        return std::make_shared<ConfigValue>(val.get<std::string>());
    }

    std::shared_ptr<ConfigNode> from_json_impl(const JSON &json)
    {
        if(json.is_object())
        {
            auto ret = std::make_shared<ConfigGroup>();
            for(auto it = json.begin(); it != json.end(); ++it)
            {
                const auto &val = it.value();
                std::shared_ptr<ConfigNode> child;
                if(!val.is_array() && !val.is_object())
                    child = get_child(val);
                else
                    child = from_json_impl(val);
                ret->insert_child(it.key(), std::move(child));
            }
            return ret;
        }

        assert(json.is_array());
        auto ret = std::make_shared<ConfigArray>();
        for(auto &elem : json)
        {
            std::shared_ptr<ConfigNode> child;
            if(!elem.is_array() && !elem.is_object())
                child = get_child(elem);
            else
                child = from_json_impl(elem);
            ret->push_back(std::move(child));
        }
        return ret;
    }
    
} // namespace anonymous

AGZ_TRACER_BEGIN

JSON string_to_json(const std::string &str)
{
    return JSON::parse(str);
}
    
JSON config_to_json(const Config &config)
{
    return to_json_impl(config);
}

std::string json_to_string(const JSON &json)
{
    return json.dump(4);
}

Config json_to_config(const JSON &json)
{
    assert(json.is_object());
    return from_json_impl(json)->as_group();
}

AGZ_TRACER_END
