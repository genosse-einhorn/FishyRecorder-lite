#include "backend.h"

#include <cmath>

static QString strAlError(ALenum err)
{
    return QString("OpenAL Error 0x%1: %2").arg(qint64(err), 8, 16, QChar('0')).arg(QString::fromUtf8(alGetString(err)));
}

namespace Recording {

Backend::Backend(QObject *parent) : QObject(parent)
{
}

Backend::~Backend()
{
    closeRecordDevice();
    closePlaybackDevice();
}

qint64 Backend::retrieveRecordedSamples(float *buffer, qint64 numSamples)
{
    // No device opened -> never retrieve anything. FIXME: Make error?
    if (!m_recordDevice)
        return 0;

    ALCint numAvailableSamples = 0;

    alcGetIntegerv(m_recordDevice, ALC_CAPTURE_SAMPLES, 1, &numAvailableSamples);

    numSamples = std::min(qint64(numAvailableSamples), numSamples);

    alcGetError(m_recordDevice);
    alcCaptureSamples(m_recordDevice, buffer, ALCsizei(numSamples));
    auto err = alcGetError(m_recordDevice);
    if (err != AL_NO_ERROR)
    {
        emit error(strAlError(err));
        return 0;
    }

    if (numSamples > 0)
        emit error(QString());

    return numSamples;
}

void Backend::submitSamplesForPlayback(float *buffer, qint64 numSamples)
{
    if (!m_playbackContext || !m_playbackDevice)
        return;

    alcGetError(m_playbackDevice);
    alcMakeContextCurrent(m_playbackContext);
    auto err = alcGetError(m_playbackDevice);
    if (err != AL_NO_ERROR)
    {
        emit error(QString("alcMakeContextCurrent: %1").arg(strAlError(err)));
        return;
    }

    // recover played buffers
    ALint numAvailBuffers = 0;
    alGetSourcei(m_playbackSource, AL_BUFFERS_PROCESSED, &numAvailBuffers);
    if (numAvailBuffers > 0)
    {
        ALuint bufferTmp[NUM_PLAYBACK_BUFFERS];
        alSourceUnqueueBuffers(m_playbackSource, numAvailBuffers, bufferTmp);
        for (ALint i = 0; i < numAvailBuffers; ++i)
        {
            m_availableBuffers.push(bufferTmp[i]);
        }
    }

    // fill a new buffer with the data
    if (m_availableBuffers.empty())
    {
        emit error(tr("Buffer overflow! Cannot play back any more audio."));
        return;
    }

    ALuint buf = m_availableBuffers.front();

    err = alcGetError(m_playbackDevice);
    alBufferData(buf, AL_FORMAT_STEREO_FLOAT32, buffer, numSamples*sizeof(float)*2, SAMPLE_RATE);
    err = alcGetError(m_playbackDevice);
    if (err != AL_NO_ERROR)
    {
        emit error(tr("While filling playback buffer: %1").arg(strAlError(err)));
        return;
    }

    alSourceQueueBuffers(m_playbackSource, 1, &buf);
    err = alcGetError(m_playbackDevice);
    if (err != AL_NO_ERROR)
    {
        emit error(tr("Queueing playback buffer: %1").arg(strAlError(err)));
        return;
    }

    m_availableBuffers.pop();

    // restart the source if needed
    ALint state = 0;
    alGetSourcei(m_playbackSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
    {
        alSourcePlay(m_playbackSource);
    }

    emit error(QString());
}

void Backend::openRecordDevice(const QString &device)
{
    if (m_recordDevice)
    {
        closeRecordDevice();
    }

    m_recordDevice = alcCaptureOpenDevice(device.size() > 0 ? device.toUtf8().data() : nullptr,
                                          SAMPLE_RATE,
                                          AL_FORMAT_STEREO_FLOAT32,
                                          SAMPLE_RATE*sizeof(float));
    if (!m_recordDevice)
    {
        emit error(tr("While opening capture device: %1").arg(strAlError(alGetError())));
        return;
    }

    auto err = alcGetError(m_recordDevice);
    alcCaptureStart(m_recordDevice);
    err = alcGetError(m_recordDevice);
    if (err != AL_NO_ERROR)
        emit error(tr("While starting capture: %1").arg(strAlError(err)));

    emit error(QString());
}

void Backend::openPlaybackDevice(const QString &device)
{
    closePlaybackDevice();

    m_playbackDevice = alcOpenDevice(device.size() > 0 ? device.toUtf8().data() : nullptr);
    if (!m_playbackDevice)
    {
        emit error(tr("While opening playback device: %1").arg(strAlError(alGetError())));
        return;
    }

    m_playbackContext = alcCreateContext(m_playbackDevice, nullptr);
    if (!m_playbackContext)
    {
        emit error(tr("While creating playback context: %1").arg(strAlError(alcGetError(m_playbackDevice))));
        closePlaybackDevice();
        return;
    }

    alcMakeContextCurrent(m_playbackContext);
    alGetError();
    alGenSources(1, &m_playbackSource);
    auto err = alGetError();
    if (err != AL_NO_ERROR)
    {
        emit error(tr("While creating playback source: %1").arg(strAlError(err)));
        closePlaybackDevice();
        return;
    }

    if (m_registeredBuffers.size() < NUM_PLAYBACK_BUFFERS)
    {
        m_registeredBuffers.resize(NUM_PLAYBACK_BUFFERS);
        alGetError();
        alGenBuffers(NUM_PLAYBACK_BUFFERS, &m_registeredBuffers[0]);
        auto err = alGetError();
        if (err != AL_NO_ERROR)
        {
            m_registeredBuffers.resize(0);
            emit error(tr("While creating playback buffers: %1").arg(strAlError(err)));
            closePlaybackDevice();
            return;
        }

        for (auto b : m_registeredBuffers)
        {
            m_availableBuffers.push(b);
        }
    }

    emit error(QString());
}

void Backend::closeRecordDevice()
{
    if (m_recordDevice)
    {
        alcCaptureStop(m_recordDevice);
        alcCaptureCloseDevice(m_recordDevice);
        m_recordDevice = nullptr;
    }
}

void Backend::closePlaybackDevice()
{
    alcMakeContextCurrent(m_playbackContext);
    if (m_playbackSource != 0)
        alDeleteSources(1, &m_playbackSource);
    if (m_registeredBuffers.size())
        alDeleteBuffers(m_registeredBuffers.size(), &m_registeredBuffers[0]);
    alcMakeContextCurrent(nullptr);
    if (m_playbackContext)
        alcDestroyContext(m_playbackContext);
    if (m_playbackDevice)
        alcCloseDevice(m_playbackDevice);

    m_playbackSource = 0;
    m_playbackContext = nullptr;
    m_playbackDevice = nullptr;
    m_registeredBuffers.resize(0);
    m_availableBuffers = std::queue<ALuint>();
}

QStringList Backend::availableRecordingDevices()
{
    QStringList retval;

    const char *devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (devices)
    {
        for (const char *dev = devices; dev[0] != 0; dev += strlen(dev)+1)
        {
            retval << QString::fromUtf8(dev);
        }
    }

    return retval;
}

QStringList Backend::availablePlaybackDevices()
{
    QStringList retval;

    const char *devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    if (devices)
    {
        for (const char *dev = devices; dev[0] != 0; dev += strlen(dev)+1)
        {
            retval << QString::fromUtf8(dev);
        }
    }

    return retval;
}

} // namespace Recording
