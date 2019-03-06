#pragma once

#include <string>
#include <type_traits>
#include <QWidget>

#include <AGZUtils/Config/Config.h>

template<typename TWidget, std::enable_if_t<std::is_base_of_v<QWidget, TWidget>, int> = 0>
class AutoDeletedQWidgetPtr
{
    TWidget *qWidget_ = nullptr;

public:

    ~AutoDeletedQWidgetPtr()
    {
        if(qWidget_ && !qWidget_->parentWidget())
            delete qWidget_;
    }

    QWidget *GetWidget() const { return qWidget_; }
};

class FilmFilterInstance
{
public:

    virtual ~FilmFilterInstance() = default;

    virtual std::string Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigNode &node) = 0;

    virtual QWidget *GetWidget() = 0;
};
