#ifndef RECORDINGCONFIGURATORPANE_H
#define RECORDINGCONFIGURATORPANE_H

#include <QWidget>

class QSettings;

namespace Recording {

class Coordinator;

namespace Ui {
class RecordingConfiguratorPane;
}

class ConfiguratorPane : public QWidget
{
    Q_OBJECT

public:
    explicit ConfiguratorPane(QWidget *parent = 0);
    ~ConfiguratorPane();

    // Hooks up signals and slots and syncs the current state into the coordinator
    void hookupCoordinator(Coordinator *c);

signals:
    void recordingDevChanged(const QString &device);
    void monitorDevChanged(const QString &device);
    void volumeChanged(float factor);
    void outputDirChanged(const QString &fdir);
    void fileFormatChanged(const QString &format);
    void mp3ArtistChanged(const QString &name);

private slots:
    void cbRecordDevChanged();
    void cbMonitorDevChanged();
    void cbFileFormatChanged();
    void slVolumeChanged();
    void outputDirButtonClick();
    void eMp3ArtistTextChanged();

private:
    Ui::RecordingConfiguratorPane *ui;
};

} // namespace Recording

#endif // RECORDINGCONFIGURATORPANE_H
