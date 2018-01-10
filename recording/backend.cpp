#include "backend.h"

#include <cmath>

static QString getAlErrorStr()
{
    ALenum err = alGetError();
    if (err == AL_NO_ERROR)
        return QString();

    const char *errstr = alGetString(err);
    if (errstr)
        return QString("OpenAL Error 0x%1: %2").arg(qint64(err), 8, 16, QChar('0')).arg(errstr);

    return QString("Unknown OpenAL Error 0x%1").arg(qint64(err), 8, 16, QChar('0'));
}

static QString getAlcErrorStr(ALCdevice *dev)
{
    ALenum err = alcGetError(dev);
    if (err == AL_NO_ERROR)
        return QString();

    const char *errstr = alcGetString(dev, err);
    if (errstr)
        return QString("OpenAL Error 0x%1: %2").arg(qint64(err), 8, 16, QChar('0')).arg(errstr);

    return QString("Unknown OpenAL Error 0x%1").arg(qint64(err), 8, 16, QChar('0'));
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
    auto err = getAlcErrorStr(m_recordDevice);
    if (err.size())
    {
        emit error(QString("alcCaptureSamples: %1").arg(err));
        return 0;
    }

    return numSamples;
}

void Backend::submitSamplesForPlayback(float *buffer, qint64 numSamples)
{
    if (!m_playbackContext || !m_playbackDevice)
        return;

    alcGetError(m_playbackDevice);
    alcMakeContextCurrent(m_playbackContext);
    auto err = getAlcErrorStr(m_playbackDevice);
    if (err.size())
    {
        emit error(QString("alcMakeContextCurrent: %1").arg(err));
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

    alGetError();
    alBufferData(buf, AL_FORMAT_STEREO_FLOAT32, buffer, numSamples*sizeof(float)*2, SAMPLE_RATE);
    err = getAlErrorStr();
    if (err.size())
    {
        emit error(QString("alBufferData: %1").arg(err));
        return;
    }

    alSourceQueueBuffers(m_playbackSource, 1, &buf);
    err = getAlErrorStr();
    if (err.size())
    {
        emit error(QString("alSourceQueueBuffers: %1").arg(err));
        return;
    }

    m_availableBuffers.pop();

    // restart the source if needed
    ALint state = 0;
    alGetSourcei(m_playbackSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
    {
        alGetError();
        alSourcePlay(m_playbackSource);
        err = getAlErrorStr();
        if (err.size())
        {
            emit error(QString("alSourcePlay: %1").arg(err));
        }
    }
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
        emit error(QString("alcCaptureOpenDevice: %1").arg(getAlcErrorStr(nullptr)));
        return;
    }

    alcGetError(m_recordDevice);
    alcCaptureStart(m_recordDevice);
    auto err = getAlcErrorStr(m_recordDevice);
    if (err.size())
    {
        emit error(QString("alcCaptureStart: %1").arg(err));
        closeRecordDevice();
        return;
    }

    if (m_playbackContext)
        emit error(QString());
}

void Backend::openPlaybackDevice(const QString &device)
{
    closePlaybackDevice();

    m_playbackDevice = alcOpenDevice(device.size() > 0 ? device.toUtf8().data() : nullptr);
    if (!m_playbackDevice)
    {
        emit error(QString("alcOpenDevice: %1").arg(getAlcErrorStr(nullptr)));
        return;
    }

    m_playbackContext = alcCreateContext(m_playbackDevice, nullptr);
    if (!m_playbackContext)
    {
        emit error(QString("alcCreateContext: %1").arg(getAlcErrorStr(m_playbackDevice)));
        closePlaybackDevice();
        return;
    }

    alcMakeContextCurrent(m_playbackContext);
    alGetError();
    alGenSources(1, &m_playbackSource);
    auto err = getAlErrorStr();
    if (err.size())
    {
        emit error(QString("alGenSources: %1").arg(err));
        closePlaybackDevice();
        return;
    }

    if (m_registeredBuffers.size() < NUM_PLAYBACK_BUFFERS)
    {
        m_registeredBuffers.resize(NUM_PLAYBACK_BUFFERS);
        alGetError();
        alGenBuffers(NUM_PLAYBACK_BUFFERS, &m_registeredBuffers[0]);
        err = getAlErrorStr();
        if (err.size())
        {
            m_registeredBuffers.resize(0);
            emit error(QString("alGenBuffers: %1").arg(err));
            closePlaybackDevice();
            return;
        }

        for (auto b : m_registeredBuffers)
        {
            m_availableBuffers.push(b);
        }
    }

    if (m_recordDevice)
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

QString Backend::debugInfo()
{
    QStringList info;

    info << QString("Available playback devices: %1").arg(availablePlaybackDevices().join(", "));
    info << QString("Available capture devices: %1").arg(availableRecordingDevices().join(", "));
    info << QString();
    info << QString("*** default device info ***");
    ALCint major, minor;
    ALCdevice *device = alcOpenDevice(nullptr);
    if (!device)
        info << QString("ERROR while opening default device: %1").arg(getAlcErrorStr(nullptr));

    const char *devname = alcGetString(device, ALC_DEVICE_SPECIFIER);
    if (devname)
        info << QString("Default Device: %1").arg(QString::fromUtf8(devname));
    else
        info << QString("ERROR while retrieving default device name: %1").arg(getAlcErrorStr(device));

    alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &major);
    alcGetIntegerv(device, ALC_MINOR_VERSION, 1, &minor);
    QString err = getAlcErrorStr(device);
    if (!err.size())
        info << QString("ALC version: %1.%2").arg(major).arg(minor);

    const char *alcExtensions = alcGetString(device, ALC_EXTENSIONS);
    if (alcExtensions)
    {
        QString extList = QString("ALC Extensions:");
        for (const char *e = alcExtensions; *e; e += strlen(e)+1)
        {
            extList = QString("%1 %2").arg(extList).arg(QString::fromUtf8(e));
        }
        info << extList;
    }

    ALCcontext *context = alcCreateContext(device, nullptr);
    if (!context)
        info << QString("ERROR while creating context: %1").arg(getAlcErrorStr(device));

    if (alcMakeContextCurrent(context) == ALC_FALSE)
        info << QString("ERROR while setting current context: %1").arg(getAlcErrorStr(device));

    info << QString("AL vendor: %1").arg(QString::fromUtf8(alGetString(AL_VENDOR)));
    info << QString("AL renderer: %1").arg(QString::fromUtf8(alGetString(AL_RENDERER)));
    info << QString("AL version: %1").arg(QString::fromUtf8(alGetString(AL_VERSION)));

    const char *alExtensions = alGetString(AL_EXTENSIONS);
    if (alExtensions)
    {
        QString extList = QString("AL extensions:");
        for (const char *e = alExtensions; *e; e += strlen(e)+1)
        {
            extList = QString("%1 %2").arg(extList).arg(QString::fromUtf8(e));
        }
        info << extList;
    }

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return info.join("\n");
}

} // namespace Recording
