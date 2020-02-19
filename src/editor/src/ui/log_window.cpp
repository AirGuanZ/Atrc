#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include <agz/editor/ui/log_widget.h>

AGZ_EDITOR_BEGIN

void LogWidgetSink::sink_it_(const spdlog::details::log_msg &msg)
{
    if(!log_widget)
        return;
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    log_widget->info(QString::fromStdString(fmt::to_string(formatted)));
}

void LogWidgetSink::flush_()
{
    
}

LogWidget::LogWidget(QWidget *parent, int max_char_count)
    : QWidget(parent), max_char_count_(max_char_count)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *clear = new QPushButton("Clear", this);
    text_ = new QPlainTextEdit(this);

    text_->setReadOnly(true);

    layout->addWidget(text_);
    layout->addWidget(clear);

    setContentsMargins(0, 0, 0, 0);
    layout->setContentsMargins(0, 0, 0, 0);

    connect(clear, &QPushButton::clicked, [=]
    {
        text_->setPlainText("");
    });
}

void LogWidget::info(const QString &msg)
{
    QString text = text_->toPlainText() + msg;
    if(text.length() > max_char_count_)
        text = text.right(max_char_count_);
    text_->setPlainText(text);

    text_->verticalScrollBar()->setValue(
        text_->verticalScrollBar()->maximum());
}

void LogWidget::warning(const QString &msg)
{
    info(msg);
}

void LogWidget::error(const QString &msg)
{
    info(msg);
}

AGZ_EDITOR_END
