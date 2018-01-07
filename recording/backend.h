#ifndef BACKEND_H
#define BACKEND_H

#include <queue>
#include <vector>
#include <QObject>
#include <QString>
#include <QStringList>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

namespace Recording {

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    static const int SAMPLE_RATE = 44100;
    static const int NUM_PLAYBACK_BUFFERS = 16;

    qint64 retrieveRecordedSamples(float *buffer, qint64 numSamples);
    void submitSamplesForPlayback(float *buffer, qint64 numSamples);

    void openRecordDevice(const QString &device);
    void openPlaybackDevice(const QString &device);
    void closeRecordDevice();
    void closePlaybackDevice();

    static QStringList availableRecordingDevices();
    static QStringList availablePlaybackDevices();

signals:
    void error(const QString &msg);

private:
    ALCdevice *m_recordDevice { nullptr };
    ALCdevice *m_playbackDevice { nullptr };
    ALCcontext *m_playbackContext { nullptr };
    ALuint m_playbackSource { 0 };
    std::queue<ALuint> m_availableBuffers {};
    std::vector<ALuint> m_registeredBuffers {};
};

} // namespace Recording

#endif // BACKEND_H
