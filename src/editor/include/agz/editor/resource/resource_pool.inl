#pragma once

AGZ_EDITOR_BEGIN

template<typename TracerObject>
ResourceReference<TracerObject>::ResourceReference(ResourceInPool<TracerObject> *rsc_in_pool)
    : rsc_in_pool_(rsc_in_pool)
{
    assert(rsc_in_pool);
    name_ = rsc_in_pool_->get_name();
}

template<typename TracerObject>
ResourceReference<TracerObject>::~ResourceReference()
{
    if(rsc_in_pool_)
        rsc_in_pool_->remove_reference(this);
    if(name_label_)
        delete name_label_.data();
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_removed_callback(std::function<void()> callback)
{
    removed_callback_ = std::move(callback);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceReference<TracerObject>::get_tracer_object()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_->get_tracer_object();
}

template<typename TracerObject>
ResourceInPool<TracerObject> *ResourceReference<TracerObject>::get_resource()
{
    assert(rsc_in_pool_);
    return rsc_in_pool_;
}

template<typename TracerObject>
QWidget *ResourceReference<TracerObject>::get_name_widget()
{
    if(name_label_)
        return name_label_;
    name_label_ = new QLabel(name_);
    name_label_->setAlignment(Qt::AlignCenter);
    return name_label_;
}

template<typename TracerObject>
std::shared_ptr<tracer::ConfigNode> ResourceReference<TracerObject>::to_config() const
{
    auto grp = std::make_shared<tracer::ConfigGroup>();
    auto name = rsc_in_pool_->get_config_ref_name();
    grp->insert_str("type", "reference");
    grp->insert_child("name", name);
    return grp;
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceReference<TracerObject>::get_vertices() const
{
    return rsc_in_pool_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceReference<TracerObject>::get_transform() const
{
    return rsc_in_pool_->get_transform();
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_transform(const DirectTransform &transform)
{
    rsc_in_pool_->set_transform(transform);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::set_name(const QString &name)
{
    name_ = name;
    if(name_label_)
        name_label_->setText(name);
}

template<typename TracerObject>
void ResourceReference<TracerObject>::call_dirty_callback()
{
    if(dirty_callback_)
        dirty_callback_();
}

template<typename TracerObject>
void ResourceReference<TracerObject>::call_remove_callback()
{
    if(removed_callback_)
        removed_callback_();
}

template<typename TracerObject>
ResourceInPool<TracerObject>::ResourceInPool(const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel)
    : name_(name)
{
    panel_ = panel.release();
    panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
    panel_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
        for(auto ref : references_)
            ref->set_geometry_vertices_dirty();
    });
    panel_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
        for(auto ref : references_)
            ref->set_entity_transform_dirty();
    });

    size_t my_ptr = reinterpret_cast<size_t>(this);
    str_ptr_ = std::to_string(my_ptr);
}

template<typename TracerObject>
ResourceInPool<TracerObject>::ResourceInPool(
    const QString &name,
    const ResourceWidgetFactory<TracerObject> &factory, const QString &default_type)
    : name_(name)
{
    panel_ = new ResourcePanel<TracerObject>(factory, default_type);
    panel_->set_dirty_callback([=]
    {
        if(dirty_callback_)
            dirty_callback_();
        for(auto ref : references_)
            ref->call_dirty_callback();
    });
    panel_->set_geometry_vertices_dirty_callback([=]
    {
        set_geometry_vertices_dirty();
        for(auto ref : references_)
            ref->set_geometry_vertices_dirty();
    });
    panel_->set_entity_transform_dirty_callback([=]
    {
        set_entity_transform_dirty();
        for(auto ref : references_)
            ref->set_entity_transform_dirty();
    });

    size_t my_ptr = reinterpret_cast<size_t>(this);
    str_ptr_ = std::to_string(my_ptr);
}

template<typename TracerObject>
ResourceInPool<TracerObject>::~ResourceInPool()
{
    while(!references_.empty())
    {
        auto ref = *references_.begin();
        ref->call_remove_callback();
        delete ref;
    }

    ResourcePanel<TracerObject> *panel = panel_;
    delete panel;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_dirty_callback(std::function<void()> callback)
{
    dirty_callback_ = std::move(callback);
}

template<typename TracerObject>
std::unique_ptr<ResourceReference<TracerObject>> ResourceInPool<TracerObject>::create_reference()
{
    auto ret = std::make_unique<ResourceReference<TracerObject>>(this);
    references_.insert(ret.get());
    return ret;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::remove_reference(ResourceReference<TracerObject> *ref)
{
    ref->rsc_in_pool_ = nullptr;
    references_.erase(ref);
}

template<typename TracerObject>
std::shared_ptr<TracerObject> ResourceInPool<TracerObject>::get_tracer_object()
{
    return panel_->get_tracer_object();
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourceInPool<TracerObject>::get_panel()
{
    return panel_;
}

template<typename TracerObject>
ResourcePanel<TracerObject> *ResourceInPool<TracerObject>::clone_panel()
{
    return panel_->clone();
}

template<typename TracerObject>
std::unique_ptr<ResourceThumbnailProvider> ResourceInPool<TracerObject>::get_thumbnail(int width, int height) const
{
    assert(panel_);
    return panel_->get_thumbnail(width, height);
}

template<typename TracerObject>
const QString &ResourceInPool<TracerObject>::get_name() const
{
    return name_;
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_name(const QString &name)
{
    name_ = name;
    for(auto ref : references_)
        ref->set_name(name);
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::save_asset(AssetSaver &saver) const
{
    saver.write_string(name_);
    panel_->save_asset(saver);
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::load_asset(ResourcePool<TracerObject> &pool, AssetLoader &loader)
{
    // loaded name can be valid
    // in this case, a new name is asked from the pool, and 'loaded_name -> new_name' is recorded by the loader

    assert(references_.empty() && !dirty_callback_);
    name_ = loader.read_string();
    if(!pool.is_valid_name(name_))
    {
        const QString new_name = pool.to_valid_name(name_);
        loader.add_rsc_name_map<TracerObject>(name_, new_name);
        name_ = new_name;
    }

    panel_->load_asset(loader);
}

template<typename TracerObject>
std::shared_ptr<tracer::ConfigNode> ResourceInPool<TracerObject>::to_config(JSONExportContext &ctx) const
{
    return panel_->to_config(ctx);
}

template<typename TracerObject>
std::shared_ptr<tracer::ConfigArray> ResourceInPool<TracerObject>::get_config_ref_name() const
{
    auto arr = std::make_shared<tracer::ConfigArray>();

    arr->push_back_str("pool");
    arr->push_back_str(
        typeid(TracerObject).name() + std::string(".") +
        name_.toStdString() + "." + str_ptr_);

    return arr;
}

template<typename TracerObject>
std::vector<EntityInterface::Vertex> ResourceInPool<TracerObject>::get_vertices() const
{
    return panel_->get_vertices();
}

template<typename TracerObject>
DirectTransform ResourceInPool<TracerObject>::get_transform() const
{
    return panel_->get_transform();
}

template<typename TracerObject>
void ResourceInPool<TracerObject>::set_transform(const DirectTransform &transform)
{
    panel_->set_transform(transform);
}

AGZ_EDITOR_END
