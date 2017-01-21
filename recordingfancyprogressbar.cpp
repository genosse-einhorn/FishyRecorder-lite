#include "recordingfancyprogressbar.h"

#include <QPainter>
#include <cmath>

RecordingFancyProgressBar::RecordingFancyProgressBar(QWidget *parent) :
    QWidget(parent)
{
}

void RecordingFancyProgressBar::setValue(float value)
{
    m_value = value;

    update();
}


QSize RecordingFancyProgressBar::sizeHint() const
{
    return QSize(50, fontMetrics().height() + 2);
}

QSize RecordingFancyProgressBar::minimumSizeHint() const
{
    return QSize(10, fontMetrics().height() + 2);
}

void RecordingFancyProgressBar::paintEvent(QPaintEvent *)
{
    int boxWidth = (int)(width() * std::pow(m_value, 1.0f/3.0f));

    QPainter painter(this);

    if (boxWidth > width()) {
        painter.fillRect(0, 0, width(), height(), palette().brush(palette().currentColorGroup(), QPalette::Highlight));
    } else {
        painter.fillRect(0, 0, width(), height(), palette().brush(palette().currentColorGroup(), QPalette::Background));
        painter.fillRect(0, 0, boxWidth, height(), palette().brush(palette().currentColorGroup(), QPalette::Foreground));
    }
}
