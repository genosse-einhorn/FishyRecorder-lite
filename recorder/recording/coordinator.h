#ifndef RECORDINGCOORDINATOR_H
#define RECORDINGCOORDINATOR_H

#include <QObject>

#include <atomic>
#include <memory>

class QIODevice;
class QTimer;
class QFile;

struct SoundIo;
struct SoundIoDevice;
struct SoundIoInStream;
struct SoundIoOutStream;
struct SoundIoRingBuffer;

namespace Recording {

class LameEncoderStream;
class LevelCalculator;

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

    static bool isSupportedInput(SoundIoDevice *dev);
    static bool isSupportedOutput(SoundIoDevice *dev);

    // A string which identifies a device uniquely, unlike SoundIoDevice::id
    // The same device will get different ids for input, output and raw modes
    static QString uniqueDeviceId(SoundIoDevice *dev);

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
    static void read_callback(struct SoundIoInStream *instream, int frame_count_min, int frame_count_max);
    static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
    static void error_callback_in(struct SoundIoInStream *instream, int err);
    static void error_callback_out(struct SoundIoOutStream *outstream, int err);

    void stopAudioInput();
    void startAudioInput();
    void stopMonitorOutput();
    void startMonitorOutput();

    void processAudio();

private:
    SoundIo *m_soundio { nullptr };

    Recording::LevelCalculator *m_levelCalculator;

    QIODevice *m_mp3FileStream { nullptr };
    Recording::LameEncoderStream *m_mp3Stream { nullptr };

    QString m_recordingDevId;
    QString m_monitorDevId;

    std::atomic<float> m_volumeFactor { 1.0f };

    std::atomic_bool m_monitorEnabled { false };

    qint64 m_samplesSaved { 0 };

    QString m_saveDir;
    QString m_filename;
    QString m_mp3ArtistName { "Someone" };

    SoundIoInStream *m_audioInStream { nullptr };
    SoundIoOutStream *m_audioOutStream { nullptr };

    SoundIoRingBuffer *m_recordRingBuffer { nullptr };
    SoundIoRingBuffer *m_monitorRingBuffer { nullptr };
};

} // namespace Recording

#endif // RECORDINGCOORDINATOR_H
