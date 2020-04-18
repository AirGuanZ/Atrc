#pragma once

#include <optional>

#include <QTextStream>
#include <QValidator>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class Vec3Validator : public QValidator
{
public:

    static std::optional<Vec3> try_parse(QString &text)
    {
        Vec3 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y >> vec.z;
        if(text_stream.status() != text_stream.Ok)
            return std::nullopt;
        return vec;
    }

    static Vec3 parse(QString &text)
    {
        Vec3 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y >> vec.z;
        return vec;
    }

    State validate(QString &input, int &pos) const override
    {
        auto vec = try_parse(input);
        if(!vec)
            return Intermediate;
        return Acceptable;
    }
};

class Vec2Validator : public QValidator
{
public:

    static std::optional<Vec2> try_parse(QString &text)
    {
        Vec2 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y;
        if(text_stream.status() != text_stream.Ok)
            return std::nullopt;
        return vec;
    }

    static Vec2 parse(QString &text)
    {
        Vec2 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y;
        return vec;
    }

    State validate(QString &input, int &) const override
    {
        if(!try_parse(input))
            return Intermediate;
        return Acceptable;
    }
};

class RealRangeValidator : public QValidator
{
public:

    static std::optional<Vec2> try_parse(QString &text)
    {
        Vec2 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y;
        if(text_stream.status() != text_stream.Ok)
            return std::nullopt;
        if(vec.x >= vec.y)
            return std::nullopt;
        return vec;
    }

    static Vec2 parse(QString &text)
    {
        Vec2 vec;
        QTextStream text_stream(&text);
        text_stream >> vec.x >> vec.y;
        return vec;
    }

    State validate(QString &input, int &) const override
    {
        if(!try_parse(input))
            return Intermediate;
        return Acceptable;
    }
};

AGZ_EDITOR_END
