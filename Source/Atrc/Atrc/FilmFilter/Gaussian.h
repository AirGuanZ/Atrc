#pragma once

#include <Atrc/Atrc/ResourceInterface/FilmFilter.h>

#include "ui_Gaussian.h"

class GaussianCore : public QWidget
{
    Q_OBJECT

public:

    static const char *GetTypeName();

    GaussianCore();

    std::string Serialize() const;

    void Deserialize(const AGZ::ConfigNode &node);

    bool IsMultiline() const noexcept;

private:

    Ui::GaussianWidget ui_;
};

using Gaussian = WidgetCore2FilmFilterInstance<GaussianCore>;
