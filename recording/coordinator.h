#ifndef RECORDINGCOORDINATOR_H
#define RECORDINGCOORDINATOR_H

#include <QObject>

class QIODevice;
class QTimer;
class QFile;

namespace Recording {

class LameEncoderStream;
class LevelCalculator;
class Backend;

class Coordinator : public QObject
{
    Q_OBJECT
public:
    static const int SAMPLE_RATE = 44100;
    static const int SAMPLE_SIZE = 2 * sizeof(float); // 2*4 bytes

    explicit Coordinator(QObject *parent = 0);
    ~Coordinator();

    QString recordingDevice() const { return m_recordingDevId; }
    QString monitorDevice() const { return m_monitorDevId; }
    bool monitorEnabled() const { return m_monitorEnabled; }

    static QStringList availableRecordingDevices();
    static QStringList availableMonitorDevices();

    bool isRecording() const { return m_mp3Stream != nullptr; }
    qint64 samplesRecorded() const { return m_samplesSaved; }

    float volumeFactor() const { return m_volumeFactor; }

    QString saveDir() const { return m_saveDir; }
    QString fileName() const { return m_filename; }
    QString mp3ArtistName() const { return m_mp3ArtistName; }

signals:
    void error(const QString &message);

    void recordingDeviceChanged(const QString &deviceId);
    void monitorDeviceChanged(const QString &deviceId);
    void monitorEnabledChanged(bool);

    void volumeFactorChanged(float);

    void saveDirChanged(const QString &dir);
    void mp3ArtistNameChanged(const QString &name);

    void statusUpdate(float levelL, float levelR, bool isRecording, qint64 recordedSamples);
    void recordingChanged(bool isRecording);

    void recordingFileOpened(const QString &filename);

public slots:
    void setRecordingDevice(const QString &deviceId);
    void setMonitorDevice(const QString &deviceId);
    void setMonitorEnabled(bool);

    void startNewTrack();
    void startRecording();
    void stopRecording();
    void setRecording(bool record);

    void setVolumeFactor(float factor);

    void handleLevelUpdate(float levelL, float levelR);

    void setSaveDir(const QString &dir);
    void setMp3ArtistName(const QString &name);

private:
    void stopAudioInput();
    void startAudioInput();
    void stopMonitorOutput();
    void startMonitorOutput();

    void processAudio();

private:
    Recording::Backend *m_backend;

    Recording::LevelCalculator *m_levelCalculator;

    QIODevice *m_mp3FileStream { nullptr };
    Recording::LameEncoderStream *m_mp3Stream { nullptr };

    QString m_recordingDevId;
    QString m_monitorDevId;

    float m_volumeFactor { 1.0f };
    bool m_monitorEnabled { false };

    qint64 m_samplesSaved { 0 };

    QString m_saveDir;
    QString m_filename;
    QString m_mp3ArtistName { "Someone" };
};

} // namespace Recording

#endif // RECORDINGCOORDINATOR_H
