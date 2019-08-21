#include <mutex>
#include <thread>

#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/utility/file/file_raw.h>

#include "./lua.h"
#include "./ideal_black.h"

/*
 * 编写lutmat shader的过程：
 *  1. 编写initialize，输入为config params，输出material params
 *  2. 编写shade，输入为material params和intersection，输出bsdf params
 *  3. 编写eval, sample, pdf, albedo, is_delta, is_black系列函数
 */

AGZ_TRACER_BEGIN

namespace
{

    struct StatePerThread
    {
        sol::state state;
        sol::table params;
        sol::table vec2_metatable;
        sol::table vec3_metatable;
        sol::table coord_metatable;
    };

    sol::table to_table(const Vec3 &v, StatePerThread &state)
    {
        return state.state.create_table_with(
            "x", v.x, "y", v.y, "z", v.z, sol::metatable_key, state.vec3_metatable);
    }

    sol::table to_table(const Vec2 &v, StatePerThread &state)
    {
        return state.state.create_table_with(
            "x", v.x, "y", v.y, sol::metatable_key, state.vec2_metatable);
    }

    sol::table to_table(const Coord &c, StatePerThread &state)
    {
        auto x = to_table(c.x, state);
        auto y = to_table(c.y, state);
        auto z = to_table(c.z, state);
        return state.state.create_table_with(
            "x", std::move(x), "y", std::move(y), "z", std::move(z), sol::metatable_key, state.coord_metatable);
    }

    sol::table to_table(sol::state &state, const Sample3 &sam)
    {
        return state.create_table_with("u", sam.u, "v", sam.v, "w", sam.w);
    }

    class LuaBSDF : public BSDF
    {
        StatePerThread &state_;
        mutable sol::table params_;

    public:

        explicit LuaBSDF(StatePerThread &state, sol::table &&params) noexcept
            : state_(state), params_(std::move(params))
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
                auto wi_tab = to_table(wi, state_);
                auto wo_tab = to_table(wo, state_);

                bool is_importance = mode == TM_Importance;
                std::tuple<real, real, real> ret = state_.state["eval"](params_, wi_tab, wo_tab, is_importance);
                return { std::get<0>(ret), std::get<1>(ret), std::get<2>(ret) };
            }
            catch(...)
            {
                return {};
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
                auto wo_tab = to_table(wo, state_);
                bool is_importance = mode == TM_Importance;
                auto sam_tab = to_table(state_.state, sam);

                sol::table output = state_.state["sample"](params_, wo_tab, is_importance, sam_tab);

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
                auto wi_tab = to_table(wi, state_);
                auto wo_tab = to_table(wo, state_);

                bool is_importance = mode == TM_Importance;
                real ret = state_.state["pdf"](params_, wi_tab, wo_tab, is_importance);
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
                sol::table tab = state_.state["albedo"](params_);
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
                bool ret = state_.state["is_delta"](params_);
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
                bool ret = state_.state["is_black"](params_);
                return ret;
            }
            catch(...)
            {
                return false;
            }
        }
    };
}

/**
 * lua script:
 *  initialize(config_table) -> material_params_table
 *  shade(material_params_table, inct_table) -> bsdf_params_table
 *  eval, sample, pdf, albedo, wi_proj_factor, is_delta, is_black (see LuaBSDF)
 */
class LuaMaterial : public Material
{
    sol::state mat_state_;
    sol::table mat_params_;

    sol::bytecode bytecode_;

    mutable std::mutex thread2lua_mutex_;

    // thread id -> state per thread
    mutable std::unordered_map<std::thread::id, std::unique_ptr<StatePerThread>> thread2lua_;

    StatePerThread *state_for_this_thread() const
    {
        auto id = std::this_thread::get_id();
        std::lock_guard lk(thread2lua_mutex_);
        
        auto it = thread2lua_.find(id);
        if(it != thread2lua_.end())
            return it->second.get();

        auto new_state = std::make_unique<StatePerThread>();
        new_state->state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::jit);
        new_state->state.safe_script(bytecode_.as_string_view(), sol::script_pass_on_error);
        new_state->params = lua::clone_table(mat_params_, new_state->state);

        new_state->vec2_metatable  = new_state->state["Vec2"];
        new_state->vec3_metatable  = new_state->state["Vec3"];
        new_state->coord_metatable = new_state->state["Coord"];

        auto ret = new_state.get();
        thread2lua_[id] = std::move(new_state);

        return ret;
    }

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
lua [Material]
    script_filename [string] (optional) lua script filename
    script          [string] (required only when 'script_filename' is not specified) script source code
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        // initialize bytecode
        {
            std::string src;
            if(auto node = params.find_child_value("script_filename"))
            {
                auto raw_filename = node->as_str();
                auto filename = init_ctx.path_mgr->get(raw_filename);
                src = file::read_txt_file(filename);
            }
            else
                src = params.find_child_value("script")->as_str();

            mat_state_.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::jit);
            auto lr = mat_state_.load(src);
            sol::protected_function func = lr;
            bytecode_ = func.dump();

            mat_state_.script(bytecode_.as_string_view());
        }

        // initialize material params
        {
            auto init_params = lua::to_table(mat_state_, params);
            mat_params_ = mat_state_["initialize"](init_params);
        }

        AGZ_HIERARCHY_WRAP("in initializing lua material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        try
        {
            auto state = state_for_this_thread();

            sol::table geometry_coord_tab = to_table(inct.geometry_coord, *state);
            sol::table user_coord_tab     = to_table(inct.user_coord, *state);
            sol::table uv_tab             = to_table(inct.uv, *state);

            sol::table inct_tab = state->state.create_table_with(
                "geometry_coord", geometry_coord_tab,
                "user_coord",     user_coord_tab,
                "uv",             uv_tab);

            sol::table bsdf_params = state->state["shade"](state->params, inct_tab);
            auto bsdf = arena.create<LuaBSDF>(*state, std::move(bsdf_params));

            return { bsdf };
        }
        catch(const std::exception &err)
        {
            printf("%s\n", err.what());
            return { IdealBlack::IDEAL_BLACK_BSDF_INSTANCE() };
        }
    }
};

AGZT_IMPLEMENTATION(Material, LuaMaterial, "lua")

AGZ_TRACER_END
