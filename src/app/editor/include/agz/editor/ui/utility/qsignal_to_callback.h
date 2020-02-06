#pragma once

#include <QObject>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class QSignalToCallback : public QObject
{
    Q_OBJECT

public:

    template<typename C, typename M, typename Fn>
    void connect_callback(C c, M m, Fn fn)
    {
        connect(c, m, fn);
    }
};

AGZ_EDITOR_END
