#pragma once

#include <cassert>
#include <memory>
#include <type_traits>

#include <agz/tracer/common.h>
#include <agz/tracer/utility/config.h>
#include <agz/tracer/utility/path_manager.h>
#include <agz/utility/misc.h>
#include <agz/utility/string.h>

AGZ_TRACER_BEGIN

namespace obj
{

    struct ObjectInitContext
    {
        Arena *arena                      = nullptr;
        const PathManager *path_mgr       = nullptr;
        const ConfigGroup *reference_root = nullptr;
    };

    class ObjectException : public std::runtime_error
    {
    public:

        using std::runtime_error::runtime_error;
    };

    class Object : public misc::uncopyable_t
    {
    public:

        virtual void initialize(const Config&, ObjectInitContext&)
        {
            
        }

        virtual ~Object() = default;
    };

    template<typename T>
    class ObjectCreator : public misc::uncopyable_t
    {
        std::string name_;

        using desc_func_ptr_t = std::string(*)();
        desc_func_ptr_t desc_func_;

        static_assert(std::is_base_of_v<Object, T>);

    public:

        ObjectCreator(std::string name, desc_func_ptr_t desc_func) noexcept
            : name_(std::move(name)), desc_func_(desc_func)
        {
            assert(desc_func);
        }

        virtual ~ObjectCreator() = default;

        const std::string &name() const noexcept
        {
            return name_;
        }

        std::string description() const
        {
            return stdstr::trim(desc_func_());
        }

        virtual T *create(const Config &params, ObjectInitContext &context) const = 0;
    };

    template<typename T>
    class ObjectFactory;

    template<typename T>
    class PoolCreator : public ObjectCreator<T>
    {
        ObjectFactory<T> *factory_;

        mutable std::map<std::vector<std::string>, T*> name2obj_;

        static std::string desc()
        {
            return R"___(
reference [Object]
    name [string[]] reference name
)___";
        }

    public:

        explicit PoolCreator(ObjectFactory<T> *factory)
            : ObjectCreator<T>("reference", &desc), factory_(factory)
        {
            
        }

        T *create(const Config &params, ObjectInitContext &context) const override;
    };

    template<typename T>
    class ObjectFactory : public misc::uncopyable_t
    {
        std::map<std::string, std::unique_ptr<ObjectCreator<T>>> creators_;

    public:

        ObjectFactory();

        void add_creator(std::unique_ptr<ObjectCreator<T>> &&creator)
        {
            assert(creator != nullptr);
            auto &name = creator->name();
            creators_[name] = std::move(creator);
        }

        const ObjectCreator<T> *find_creator(const std::string &type_name) const
        {
            auto it = creators_.find(type_name);
            return it != creators_.end() ? it->second.get() : nullptr;
        }

        T *create(const std::string &type_name, const Config &params, ObjectInitContext &context) const
        {
            if(auto creator = find_creator(type_name))
                return creator->create(params, context);
            throw ObjectException(stdstr::cat("creator not found (typename: ", type_name, ")"));
        }

        T *create(const Config &params, ObjectInitContext &context)
        {
            return create(params.child_str("type"), params, context);
        }

        auto begin()       { return creators_.begin(); }
        auto end()         { return creators_.end();   }
        auto begin() const { return creators_.begin(); }
        auto end()   const { return creators_.end();   }
    };

    template<typename T>
    T *PoolCreator<T>::create(const Config &params, ObjectInitContext &context) const
    {
        auto &name_arr = params.child_array("name");
        if(name_arr.size() < 1)
            throw ObjectConstructionException("empty reference name sequence");

        std::vector<std::string> names;
        names.reserve(name_arr.size());
        for(size_t i = 0; i < name_arr.size(); ++i)
            names.push_back(name_arr.at(i).as_value().as_str());

        if(auto it = name2obj_.find(names); it != name2obj_.end())
            return it->second;

        const ConfigGroup *group = context.reference_root;

        for(size_t i = 0; i < names.size() - 1; ++i)
            group = &group->child_group(names[i]);

        auto &true_params = group->child_group(names.back());
        auto ret = factory_->create(true_params, context);
        name2obj_[names] = ret;

        return ret;
    }

    template<typename T>
    ObjectFactory<T>::ObjectFactory()
    {
        creators_["reference"] = std::make_unique<PoolCreator<T>>(this);
    }

}

#define AGZT_INTERFACE(BaseClassName) \
    namespace obj \
    { \
        static_assert(std::is_base_of_v<::agz::tracer::obj::Object, BaseClassName>); \
        inline ObjectFactory<BaseClassName> &factory_##BaseClassName() \
        { \
            static ObjectFactory<BaseClassName> ret; \
            return ret; \
        } \
    } \
    inline auto &BaseClassName##Factory = AGZT_FACTORY(BaseClassName);

#define AGZT_FACTORY(BaseClassName) (::agz::tracer::obj::factory_##BaseClassName())

#define AGZT_ADD_CREATOR(BaseClassName, CreatorClassName) \
    namespace \
    { \
        class AGZT_Impl_Register_##CreatorClassName : public ::agz::misc::uncopyable_t \
        { \
        public: \
            AGZT_Impl_Register_##CreatorClassName() \
            { \
                AGZT_FACTORY(BaseClassName).add_creator(std::make_unique<CreatorClassName>()); \
            } \
        }; \
        AGZT_Impl_Register_##CreatorClassName impl_register##CreatorClassName; \
    }

#define AGZT_IMPLEMENTATION(BaseClassName, ImplClassName, NameStr) \
    namespace \
    { \
        class AGZT_Impl_Creator##ImplClassName : public ::agz::tracer::obj::ObjectCreator<BaseClassName> \
        { \
        public: \
            AGZT_Impl_Creator##ImplClassName(): ObjectCreator(NameStr, &ImplClassName::description) { } \
            BaseClassName *create(const Config &params, ::agz::tracer::obj::ObjectInitContext &context) const override \
            { \
                auto ret = context.arena->create<ImplClassName>(); \
                ret->initialize(params, context); \
                return ret; \
            } \
        }; \
        class AGZT_Impl_Register_##ImplClassName : public ::agz::misc::uncopyable_t \
        { \
        public: \
            AGZT_Impl_Register_##ImplClassName() \
            { \
                AGZT_FACTORY(BaseClassName).add_creator(std::make_unique<AGZT_Impl_Creator##ImplClassName>()); \
            } \
        }; \
        AGZT_Impl_Register_##ImplClassName impl_register##ImplClassName; \
    }

AGZ_TRACER_END
