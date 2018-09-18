#pragma once

#include "../Geometry.h"
#include "../Transform.h"

AGZ_NS_BEG(Atrc)

class GeometryObjectWithTransform : public GeometryObject
{
protected:

    const Transform *local2World_;

public:

    explicit GeometryObjectWithTransform(
        const Transform *local2World = &Transform::StaticIdentity())
        : local2World_(local2World)
    {
        AGZ_ASSERT(local2World_);
    }

    const Transform *GetTransform() const
    {
        return local2World_;
    }

    void SetTransform(const Transform *local2World)
    {
        AGZ_ASSERT(local2World);
        local2World_ = local2World;
    }
};

AGZ_NS_END(Atrc)
