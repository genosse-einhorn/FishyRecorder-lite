#ifndef FANCYPROGRESSBAR_H
#define FANCYPROGRESSBAR_H

#include <QProgressBar>

class RecordingFancyProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit RecordingFancyProgressBar(QWidget *parent = 0);

signals:

public slots:
    void setValue(float value);


    // QWidget interface
public:
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    float m_value = 0.0f;
};

#endif // FANCYPROGRESSBAR_H
