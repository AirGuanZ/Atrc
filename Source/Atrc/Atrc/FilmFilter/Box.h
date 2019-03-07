#pragma once

#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

#include "ui_Box.h"

class BoxCore : public QWidget
{
    Q_OBJECT

    Ui::BoxWidget ui_;

public:

    BoxCore();

    std::string Serialize() const;

    void Deserialize(const AGZ::ConfigNode &node);
};

using Box = WidgetCore2FilmFilterInstance<BoxCore>;
