#pragma once

#include <agz/tracer/core/scene.h>

AGZ_TRACER_BEGIN

/**
 * @brief 场景自动构建设施
 */
class SceneBuilder
{
public:

    /**
     * @brief 利用配置参数构建场景
     * 
     * 合法的参数应包含：
     * 
     * entities(optional)       由描述Entity的ConfigGroup构成的ConfigArray
     * named_entities(optional) 由(entity_name_string, entity ConfigGroup)构成的ConfigGroup
     * env(optional)            环境光源，包裹整个场景的光源，实际为实体类型
     */
    static Scene *build(const Config &params, obj::ObjectInitContext &context);
};

AGZ_TRACER_END
