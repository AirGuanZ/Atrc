#pragma once

#include <QFrame>
#include <QPainter>
#include <QTextLayout>

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

class ElidedLabel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool isElided READ isElided)

public:

    explicit ElidedLabel(const QString &text, QWidget *parent = 0)
        : QFrame(parent), elided(false), content(text)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    void setText(const QString &text)
    {
        content = text;
        update();
    }

    const QString &text() const { return content; }

    bool isElided() const { return elided; }

protected:

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        QFrame::paintEvent(event);

        QPainter painter(this);
        QFontMetrics fontMetrics = painter.fontMetrics();

        bool didElide = false;
        int lineSpacing = fontMetrics.lineSpacing();
        int y = 0;

        QTextLayout textLayout(content, painter.font());
        textLayout.beginLayout();
        for(;;)
        {
            QTextLine line = textLayout.createLine();

            if(!line.isValid())
                break;

            line.setLineWidth(width());
            int nextLineY = y + lineSpacing;

            if(height() >= nextLineY + lineSpacing)
            {
                line.draw(&painter, QPoint(0, y));
                y = nextLineY;
            }
            else
            {
                QString lastLine = content.mid(line.textStart());
                QString elidedLastLine = fontMetrics.elidedText(
                    lastLine, Qt::ElideRight, width());
                painter.drawText(
                    QPoint(0, y + fontMetrics.ascent()), elidedLastLine);
                line = textLayout.createLine();
                didElide = line.isValid();
                break;
            }
        }

        textLayout.endLayout();

        if(didElide != elided)
        {
            elided = didElide;
            emit elisionChanged(didElide);
        }
    }

signals:

    void elisionChanged(bool elided);

private:

    bool elided;
    QString content;
};

AGZ_EDITOR_END
