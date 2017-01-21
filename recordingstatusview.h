#ifndef RECORDINGSTATUSVIEW_H
#define RECORDINGSTATUSVIEW_H

#include <QWidget>

namespace Ui {
class RecordingStatusView;
}

class RecordingStatusView : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingStatusView(QWidget *parent = 0);
    ~RecordingStatusView();

public slots:
    void handleStatusUpdate(float levelL, float levelR, bool isRecording, qint64 sampleCount);

private:
    Ui::RecordingStatusView *ui;
};

#endif // RECORDINGSTATUSVIEW_H
