#pragma once

#include <sstream>

class Exporter
{
    std::stringstream sst_;

    void AppendAux() { }
    template<typename Arg, typename...Args>
    void AppendAux(Arg &&arg, Args&&...args)
    {
        sst_ << std::forward<Arg>(arg);
        AppendAux(std::forward<Args>(args)...);
    }

public:

    template<typename...Args>
    void Append(Args&&...args)
    {
        AppendAux(std::forward<Args>(args)...);
    }

    template<typename...Args>
    void AppendLine(Args&&...args)
    {
        Append(std::forward<Args>(args)..., "\n");
    }

    template<typename Arg>
    Exporter &operator<<(Arg &&arg)
    {
        Append(std::forward<Arg>(arg));
        return *this;
    }

    void Clear()
    {
        sst_.clear();
        sst_.str("");
    }

    std::string GetString() const
    {
        return sst_.str();
    }
};
