#pragma once

#include <map>
#include <memory>
#include <string>

#include <QWidget>

#include <AGZUtils/Config/Config.h>
#include "Atrc/Atrc/QUtils.h"

class FilmFilterInstance
{
public:

    virtual ~FilmFilterInstance() = default;

    virtual std::string Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigNode &node) = 0;

    virtual QWidget *GetWidget() = 0;
};

class FilmFilterInstanceCreator
{
public:

    virtual ~FilmFilterInstanceCreator() = default;

    virtual std::shared_ptr<FilmFilterInstance> Create() const = 0;

    static const std::map<std::string, const FilmFilterInstanceCreator*> &GetAllCreators();
};

template<typename TFilmFilterInstance>
class FilmFilterInstance2Creator : public FilmFilterInstanceCreator
{
public:

    std::shared_ptr<FilmFilterInstance> Create() const override
    {
        return std::make_shared<TFilmFilterInstance>();
    }
};

template<typename TWidgetCore>
class WidgetCore2FilmFilterInstance : public FilmFilterInstance
{
    UniqueQPtr<TWidgetCore> core_;

public:

    std::string Serialize() const override
    {
        return core_->Serialize();
    }

    void Deserialize(const AGZ::ConfigNode &node) override
    {
        core_->Deserialize(node);
    }

    QWidget *GetWidget() override
    {
        return core_.get();
    }
};
