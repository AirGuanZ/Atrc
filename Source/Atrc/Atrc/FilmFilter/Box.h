#pragma once

#include <QGroupBox>

#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>
#include "ui_Box.h"

class BoxCore : public QGroupBox
{
    Q_OBJECT

public:

    static const char *GetTypeName();

    BoxCore();

    std::string Serialize() const;

    void Deserialize(const AGZ::ConfigNode &node);

private:

    Ui::BoxWidget ui_;
};

using Box = WidgetCore2FilmFilterInstance<BoxCore>;
