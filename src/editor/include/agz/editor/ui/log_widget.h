#pragma once

#include <mutex>

#include <QPlainTextEdit>

#include <agz/editor/common.h>

#include <spdlog/sinks/base_sink.h>

AGZ_EDITOR_BEGIN

class LogWidget;

class LogWidgetSink : public spdlog::sinks::base_sink<std::mutex>
{
public:

    LogWidget *log_widget = nullptr;

protected:

    void sink_it_(const spdlog::details::log_msg &msg) override;

    void flush_() override;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:

    explicit LogWidget(
        QWidget *parent = nullptr, int max_char_count = 20 * 1024 * 1024);

    void info(const QString &msg);

    void warning(const QString &msg);

    void error(const QString &msg);

signals:

    void new_info(QString msg);

private slots:

    void on_info(QString msg);

private:

    int max_char_count_ = 0;

    QPlainTextEdit *text_ = nullptr;
};

AGZ_EDITOR_END
