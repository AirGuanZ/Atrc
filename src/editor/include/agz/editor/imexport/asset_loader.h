#pragma once

#include <istream>

#include <agz/editor/imexport/asset_consts.h>
#include <agz/editor/imexport/asset_section.h>

AGZ_EDITOR_BEGIN

class AssetLoader : public misc::uncopyable_t
{
    std::istream &fin_;

    std::vector<std::function<void()>> delayed_oprs_;

    // typeinfo(TracerObject).name() + rsc_name -> new_rsc_name
    std::map<QString, QString> rsc_name_map_;

    AssetVersion version_;

public:

    explicit AssetLoader(std::istream &fin);

    const AssetVersion &version() const noexcept;

    template<typename TracerObject>
    void add_rsc_name_map(const QString &old_name, const QString &new_name);

    template<typename TracerObject>
    QString rsc_name_map(const QString &old_name);

    void add_delayed_opr(std::function<void()> opr);

    void exec_delayed_oprs();

    void read_raw(void *data, size_t byte_size);

    QString read_string();

    template<typename T>
    T read();
};

inline const AssetVersion &AssetLoader::version() const noexcept
{
    return version_;
}

template<typename TracerObject>
void AssetLoader::add_rsc_name_map(const QString &old_name, const QString &new_name)
{
    rsc_name_map_[typeid(TracerObject).name() + old_name] = new_name;
}

template<typename TracerObject>
QString AssetLoader::rsc_name_map(const QString &old_name)
{
    const QString search_name = typeid(TracerObject).name() + old_name;
    const auto it = rsc_name_map_.find(search_name);
    if(it != rsc_name_map_.end())
        return it->second;
    return old_name;
}

inline void AssetLoader::add_delayed_opr(std::function<void()> opr)
{
    delayed_oprs_.push_back(std::move(opr));
}

inline void AssetLoader::exec_delayed_oprs()
{
    for(auto &opr : delayed_oprs_)
        opr();
    delayed_oprs_.clear();
}

inline void AssetLoader::read_raw(void *data, size_t byte_size)
{
    fin_.read(static_cast<char *>(data), byte_size);
}

inline QString AssetLoader::read_string()
{
    const int32_t len = read<int32_t>();
    QString ret(len, ' ');
    read_raw(ret.data(), len * sizeof(QChar));
    return ret;
}

template<typename T>
T AssetLoader::read()
{
    static_assert(std::is_trivially_copyable_v<T>);

    T ret;
    this->read_raw(&ret, sizeof(T));
    return ret;
}

AGZ_EDITOR_END
