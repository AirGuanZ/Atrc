#include <agz/tracer_utility/shader_c_api.h>
#include <agz/utility/misc/unreachable.h>

AGZ_TRACER_BEGIN

void cpp_to_c(const ConfigNode &node, Arena &arena, AGZTConfigNode *output)
{
    if(node.is_value())
    {
        output->type = AGZTConfigNodeType::Value;
        cpp_to_c(node.as_value(), arena, &output->value);
        return;
    }

    if(node.is_group())
    {
        output->type = AGZTConfigNodeType::Group;
        cpp_to_c(node.as_group(), arena, &output->group);
        return;
    }

    assert(node.is_array());
    output->type = AGZTConfigNodeType::Array;
    cpp_to_c(node.as_array(), arena, &output->array);
}

void cpp_to_c(const ConfigGroup &group, Arena &arena, AGZTConfigGroup *output)
{
    size_t size = group.size();

    auto keys = arena.create<std::vector<std::string>>();
    auto nodes = arena.create<std::vector<AGZTConfigNode>>();
    keys->reserve(size);
    nodes->reserve(size);

    for(auto &p : group)
    {
        keys->push_back(p.first);
        
        nodes->emplace_back();
        cpp_to_c(*p.second, arena, &nodes->back());
    }

    auto keys_entry = arena.create<std::vector<const char*>>(size);
    for(size_t i = 0; i < size; ++i)
        keys_entry->at(i) = keys->at(i).c_str();

    output->size = size;
    output->keys = keys_entry->data();
    output->nodes = nodes->data();
}

void cpp_to_c(const ConfigArray &array, Arena &arena, AGZTConfigArray *output)
{
    size_t size = array.size();
    auto elems = arena.create<std::vector<AGZTConfigNode>>(size);
    for(size_t i = 0; i < size; ++i)
        cpp_to_c(array.at(i), arena, &elems->at(i));

    output->size = size;
    output->elems = elems->data();
}

void cpp_to_c(const ConfigValue &value, Arena &arena, AGZTConfigValue *output)
{
    output->str = arena.create<std::string>(value.as_str())->c_str();
}

std::shared_ptr<ConfigNode> c_to_cpp(const AGZTConfigNode *node)
{
    switch(node->type)
    {
    case AGZTConfigNodeType::Group:
        return c_to_cpp(&node->group);
    case AGZTConfigNodeType::Array:
        return c_to_cpp(&node->array);
    case AGZTConfigNodeType::Value:
        return c_to_cpp(&node->value);
    }
    misc::unreachable();
}

std::shared_ptr<ConfigGroup> c_to_cpp(const AGZTConfigGroup *group)
{
    auto ret = std::make_shared<ConfigGroup>();
    for(size_t i = 0; i < group->size; ++i)
    {
        auto name = group->keys[i];
        auto child = c_to_cpp(&group->nodes[i]);
        ret->insert_child(name, std::move(child));
    }
    return ret;
}

std::shared_ptr<ConfigArray> c_to_cpp(const AGZTConfigArray *array)
{
    auto ret = std::make_shared<ConfigArray>();
    for(size_t i = 0; i < array->size; ++i)
    {
        auto elem = c_to_cpp(&array->elems[i]);
        ret->push_back(std::move(elem));
    }
    return ret;
}

std::shared_ptr<ConfigValue> c_to_cpp(const AGZTConfigValue *value)
{
    return std::make_shared<ConfigValue>(value->str);
}

AGZ_TRACER_END
