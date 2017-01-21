#ifndef RECORDINGCONFIGURATORPANE_H
#define RECORDINGCONFIGURATORPANE_H

#include <QWidget>

#include <portaudio.h>

class QSettings;
class RecordingCoordinator;


namespace Ui {
class RecordingConfiguratorPane;
}

class RecordingConfiguratorPane : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingConfiguratorPane(QWidget *parent = 0);
    ~RecordingConfiguratorPane();

    // Hooks up signals and slots and syncs the current state into the coordinator
    void hookupCoordinator(RecordingCoordinator *c);

signals:
    void recordingDevChanged(PaDeviceIndex i);
    void monitorDevChanged(PaDeviceIndex i);
    void volumeChanged(float factor);
    void outputDirChanged(const QString &fdir);
    void mp3ArtistChanged(const QString &name);

private slots:
    void cbRecordDevChanged();
    void cbMonitorDevChanged();
    void slVolumeChanged();
    void outputDirButtonClick();
    void eMp3ArtistTextChanged();

private:
    Ui::RecordingConfiguratorPane *ui;
};

#endif // RECORDINGCONFIGURATORPANE_H
