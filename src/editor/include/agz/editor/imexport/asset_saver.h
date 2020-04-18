#pragma once

#include <ostream>
#include <typeindex>
#include <unordered_set>

#include <agz/editor/imexport/asset_section.h>
#include <agz/tracer/core/geometry.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/medium.h>
#include <agz/tracer/core/texture3d.h>

AGZ_EDITOR_BEGIN

class AssetSaver : public misc::uncopyable_t
{
    std::ostream &fout_;

    std::unordered_set<std::type_index> enabled_pools_;

public:

    explicit AssetSaver(std::ostream &fout);

    void write_raw(const void *data, size_t byte_size);

    void write_string(const QString &str);

    template<typename T>
    void write(const T &value);

    template<typename TracerObject>
    void enable_rsc_pool();

    template<typename TracerObject>
    bool is_rsc_pool_enabled();
};

inline void AssetSaver::write_raw(const void *data, size_t byte_size)
{
    fout_.write(static_cast<const char*>(data), byte_size);
}

inline void AssetSaver::write_string(const QString &str)
{
    write<int32_t>(str.length());
    write_raw(str.data(), str.length() * sizeof(QChar));
}

template<typename T>
void AssetSaver::write(const T &value)
{
    static_assert(std::is_trivially_copyable_v<T>);
    this->write_raw(&value, sizeof(value));
}

template<typename TracerObject>
void AssetSaver::enable_rsc_pool()
{
    enabled_pools_.insert(std::type_index(typeid(TracerObject)));
}

template<typename TracerObject>
bool AssetSaver::is_rsc_pool_enabled()
{
    const auto idx = std::type_index(typeid(TracerObject));
    return enabled_pools_.find(idx) != enabled_pools_.end();
}

AGZ_EDITOR_END
