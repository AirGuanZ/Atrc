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

    virtual const char *GetName() const = 0;

    virtual std::shared_ptr<FilmFilterInstance> Create() const = 0;

    static const std::map<std::string, const FilmFilterInstanceCreator*> &GetAllCreators();
};

template<typename TFilmFilterInstance>
class FilmFilterInstance2Creator : public FilmFilterInstanceCreator
{
public:

    const char *GetName() const override
    {
        return TFilmFilterInstance::GetTypeName();
    }

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

    static const char *GetTypeName()
    {
        return TWidgetCore::GetTypeName();
    }

    WidgetCore2FilmFilterInstance()
        : core_(MakeUniqueQ<TWidgetCore>())
    {

    }

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
