#pragma once

#include <agz/tracer/core/medium.h>
#include <agz/tracer_utility/config.h>
#include "../medium/medium_void.h"

AGZ_TRACER_BEGIN

struct MediumInterface
{
    const Medium *in  = nullptr;
    const Medium *out = nullptr;

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx)
    {
        AGZ_HIERARCHY_TRY

        if(auto grp = params.find_child_group("med_in"))
            in = MediumFactory.create(*grp, init_ctx);
        else
            in = &VoidMedium::VOID_MEDIUM();

        if(auto grp = params.find_child_group("med_out"))
            out = MediumFactory.create(*grp, init_ctx);
        else
            out = &VoidMedium::VOID_MEDIUM();

        AGZ_HIERARCHY_WRAP("in initializing medium interface")
    }
};

AGZ_TRACER_END
