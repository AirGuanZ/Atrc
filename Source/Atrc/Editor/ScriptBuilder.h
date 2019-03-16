#pragma once

#include <sstream>
#include <string>

class ScriptBuilder
{
    size_t indent_ = 0;
    std::string indentStr_;
    std::stringstream sst_;

    void AddLineAux() const { }

    template<typename TStr>
    void AddLineAux(TStr &&str)
    {
        sst_ << std::forward<TStr>(str);
    }

    template<typename TStr0, typename TStr1, typename...Others>
    void AddLineAux(TStr0 &&str0, TStr1 &&str1, Others&&...others)
    {
        AddLineAux(std::forward<TStr0>(str0));
        AddLineAux(std::forward<TStr1>(str1), std::forward<Others>(others)...);
    }

public:

    void IncIndent()
    {
        indentStr_ = std::string(4 * ++indent_, ' ');
    }

    void DecIndent()
    {
        indentStr_ = std::string(4 * --indent_, ' ');
    }

    std::string GetString() const
    {
        return sst_.str();
    }

    template<typename...TStrs>
    void AddLine(TStrs&&...strs)
    {
        sst_ << indentStr_;
        AddLineAux(std::forward<TStrs>(strs)...);
        sst_ << std::endl;
    }

    void ClearString()
    {
        sst_.str("");
    }
};
