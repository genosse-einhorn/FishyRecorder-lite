#ifndef RECORDINGSTATUSVIEW_H
#define RECORDINGSTATUSVIEW_H

#include <QWidget>

namespace Recording {

namespace Ui {
class RecordingStatusView;
}

class StatusView : public QWidget
{
    Q_OBJECT

public:
    explicit StatusView(QWidget *parent = 0);
    ~StatusView();

public slots:
    void handleStatusUpdate(float levelL, float levelR, bool isRecording, qint64 sampleCount);

private:
    Ui::RecordingStatusView *ui;
};

} // namespace Recording

#endif // RECORDINGSTATUSVIEW_H
