#pragma once

#include <string>

#include <AGZUtils/Config/Config.h>

class FilmFilterInstance
{
public:

    virtual ~FilmFilterInstance() = default;

    virtual std::string Serialize() const = 0;

    virtual void Deserialize(const AGZ::ConfigNode &node) = 0;
};
