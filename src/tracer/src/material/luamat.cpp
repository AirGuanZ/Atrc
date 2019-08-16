#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer/utility/lua.h>

AGZ_TRACER_BEGIN

namespace
{

    class LuaBSDF : public BSDF
    {
        sol::state &state_;
        mutable sol::table params_;

    public:

        explicit LuaBSDF(sol::state &lua_state, sol::table &&params) noexcept
            : state_(lua_state), params_(std::move(params))
        {
            
        }

        /**
         * eval params
         *  wi: table{ x, y, z }
         *  wo: table{ x, y, z }
         *  is_importance: boolean
         * return
         *  table{ r, g, b }
         */
        Spectrum eval(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            try
            {
                auto wi_tab = lua::to_table(state_, wi);
                auto wo_tab = lua::to_table(state_, wo);
                bool is_importance = mode == TM_Importance;
                std::tuple<real, real, real> ret = state_["eval"](params_, wi_tab, wo_tab, is_importance);
                return { std::get<0>(ret), std::get<1>(ret), std::get<2>(ret) };
            }
            catch(...)
            {
                return {};
            }
        }

        /**
         * proj_wi_factor params
         *  wi: table{ x, y, z }
         * return
         *  real
         */
        real proj_wi_factor(const Vec3 &wi) const noexcept override
        {
            try
            {
                auto wi_tab = lua::to_table(state_, wi);
                real ret = state_["proj_wi_factor"](params_, wi_tab);
                return ret;
            }
            catch(...)
            {
                return 0;
            }
        }

        /**
         * sample params
         *  wo: table{ x, y, z }
         *  is_importance: boolean
         *  sam: table{u, v, w}
         * return
         *  table
         *  {
         *     dir: table{ x, y, z }
         *     f: table{ r, g, b }
         *     pdf: real
         *     is_importance: boolean
         *     is_delta: boolean
         *  }
         */
        BSDFSampleResult sample(const Vec3 &wo, TransportMode mode, const Sample3 &sam) const noexcept override
        {
            try
            {
                auto wo_tab = lua::to_table(state_, wo);
                bool is_importance = mode == TM_Importance;
                auto sam_tab = lua::to_table(state_, sam);
                sol::table output = state_["sample"](params_, wo_tab, is_importance, sam_tab);

                BSDFSampleResult ret;
                ret.dir      = lua::table_to_vec3(output["dir"]);
                ret.f        = lua::table_to_spectrum(output["f"]);
                ret.pdf      = output["pdf"];
                ret.mode     = output.get<bool>("is_importance") ? TM_Importance : TM_Radiance;
                ret.is_delta = output.get<bool>("is_delta");

                return ret;
            }
            catch(...)
            {
                return BSDF_SAMPLE_RESULT_INVALID;
            }
        }

        /**
         * pdf params
         *  wi: table{ x, y, z }
         *  wo: table{ x, y, z }
         *  is_importance: boolean
         * return
         *  real
         */
        real pdf(const Vec3 &wi, const Vec3 &wo, TransportMode mode) const noexcept override
        {
            try
            {
                auto wi_tab = lua::to_table(state_, wi);
                auto wo_tab = lua::to_table(state_, wo);
                bool is_importance = mode == TM_Importance;
                real ret = state_["pdf"](params_, wi_tab, wo_tab, is_importance);
                return ret;
            }
            catch(...)
            {
                return 0;
            }
        }

        /**
         * albedo return
         *  table{ r, g, b }
         */
        Spectrum albedo() const noexcept override
        {
            try
            {
                sol::table tab = state_["albedo"](params_);
                return lua::table_to_spectrum(tab);
            }
            catch(...)
            {
                return {};
            }
        }

        /**
         * is_delta return
         *  boolean
         */
        bool is_delta() const noexcept override
        {
            try
            {
                bool ret = state_["is_delta"](params_);
                return ret;
            }
            catch(...)
            {
                return false;
            }
        }

        /**
         * is_black return 
         *  boolean
         */
        bool is_black() const noexcept override
        {
            try
            {
                bool ret = state_["is_black"](params_);
                return ret;
            }
            catch(...)
            {
                return false;
            }
        }
    };
}

class LuaMaterial : public Material
{
public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
lua [Material]
    filename [string] lua script filename
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        // TODO

        AGZ_HIERARCHY_WRAP("in initializing lua material")
    }
};

AGZ_TRACER_END
