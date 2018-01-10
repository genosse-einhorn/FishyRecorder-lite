#include "coordinator.h"

#include "levelcalculator.h"
#include "lameencoderstream.h"
#include "backend.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QtMath>

namespace {

QString makeFilenameUnique(const QString &filename)
{
    auto i = QFileInfo(filename);
    auto n = QFileInfo(filename);
    int c = 0;
    while (n.exists()) {
        c++;
        n.setFile(QString("%1/%2 (%3).%4")
                  .arg(i.path()).arg(i.completeBaseName())
                  .arg(c).arg(i.suffix()));
    }

    return n.filePath();
}

} // anonymous namespace

namespace Recording {

Coordinator::Coordinator(QObject *parent) : QObject(parent)
{
    m_levelCalculator = new LevelCalculator(this);
    m_backend = new Backend(this);

    QObject::connect(m_levelCalculator, &LevelCalculator::levelUpdate, this, &Coordinator::handleLevelUpdate);
    QObject::connect(m_backend, &Backend::error, this, &Coordinator::error);

    QTimer *t = new QTimer(this);
    t->setInterval(40);
    QObject::connect(t, &QTimer::timeout, this, &Coordinator::processAudio);
    t->start();
}

Coordinator::~Coordinator()
{
    stopAudioInput();
    stopMonitorOutput();
}

QStringList Coordinator::availableRecordingDevices()
{
    return Backend::availableRecordingDevices();
}

QStringList Coordinator::availableMonitorDevices()
{
    return Backend::availablePlaybackDevices();
}

QString Coordinator::backendDebugInfo()
{
    return Backend::debugInfo();
}

void Coordinator::setRecordingDevice(const QString &deviceId)
{
    stopAudioInput();

    m_recordingDevId = deviceId;
    emit recordingDeviceChanged(m_recordingDevId);

    startAudioInput();
}

void Coordinator::setMonitorDevice(const QString &device)
{
    stopMonitorOutput();

    m_monitorDevId = device;
    emit monitorDeviceChanged(m_monitorDevId);

    startMonitorOutput();
}

void Coordinator::setMonitorEnabled(bool enabled)
{
    if (m_monitorEnabled != enabled)
    {
        m_monitorEnabled = enabled;
        emit monitorEnabledChanged(m_monitorEnabled);
    }
}

void Coordinator::startNewTrack()
{
    stopRecording();
    startRecording();
}

void Coordinator::startRecording()
{
    if (isRecording())
        stopRecording();

    QString filename = QString(tr("Recording from %2.mp3"))
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss"));

    QString fullFilename = makeFilenameUnique(QDir::cleanPath(QString("%1/%2").arg(m_saveDir).arg(filename)));
    m_mp3FileStream = new QFile(fullFilename);
    if (!m_mp3FileStream->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        error(tr("MP3: Could not open file %1: %2").arg(filename, m_mp3FileStream->errorString()));
        stopRecording();
        return;
    }

    QString track = tr("Recording from %1").arg(QDateTime::currentDateTime().toString(Qt::DefaultLocaleLongDate));

    m_mp3Stream = new LameEncoderStream(this);
    QObject::connect(m_mp3Stream, &LameEncoderStream::error, this, &Coordinator::error);

    if (!m_mp3Stream->init(m_mp3ArtistName, track, 192, SAMPLE_RATE, m_mp3FileStream))
    {
        stopRecording();
        return;
    }

    emit recordingChanged(isRecording());
    emit recordingFileOpened(fullFilename);
}

void Coordinator::stopRecording()
{
    if (m_mp3Stream)
    {
        m_mp3Stream->close();
        delete m_mp3Stream;
        m_mp3Stream = nullptr;
    }

    if (m_mp3FileStream)
    {
        m_mp3FileStream->close();
        delete m_mp3FileStream;
        m_mp3FileStream = nullptr;
    }

    m_samplesSaved = 0;
    emit recordingChanged(isRecording());
}

void Coordinator::setRecording(bool record)
{
    if (record)
        startRecording();
    else
        stopRecording();
}

void Coordinator::setVolumeFactor(float factor)
{
    if (m_volumeFactor != factor)
    {
        m_volumeFactor = factor;
        emit volumeFactorChanged(m_volumeFactor);
    }
}

void Coordinator::handleLevelUpdate(float levelL, float levelR)
{
    emit statusUpdate(levelL, levelR, isRecording(), samplesRecorded());
}

void Coordinator::setSaveDir(const QString &dir)
{
    if (m_saveDir != dir)
    {
        m_saveDir = dir;
        emit saveDirChanged(dir);
    }
}

void Coordinator::setMp3ArtistName(const QString &name)
{
    if (m_mp3ArtistName != name)
    {
        m_mp3ArtistName = name;
        emit mp3ArtistNameChanged(name);
    }
}

void Coordinator::stopAudioInput()
{
    m_backend->closeRecordDevice();
}

void Coordinator::startAudioInput()
{
    m_backend->openRecordDevice(m_recordingDevId);
}

void Coordinator::stopMonitorOutput()
{
    m_backend->closePlaybackDevice();
}

void Coordinator::startMonitorOutput()
{
    m_backend->openPlaybackDevice(m_monitorDevId);
}

void Coordinator::processAudio()
{
    float buffer[4096];

    for(;;)
    {
        qint64 numSamples = sizeof(buffer)/sizeof(float)/2;

        numSamples = m_backend->retrieveRecordedSamples(buffer, numSamples);
        if (numSamples <= 0)
            return;

        // apply volume changes
        for (qint64 i = 0; i < numSamples*2; ++i)
        {
            buffer[i] = buffer[i]*m_volumeFactor;
        }

        // emit monitor output
        if (m_monitorEnabled)
        {
            m_backend->submitSamplesForPlayback(buffer, numSamples);
        }

        // do level calculation
        m_levelCalculator->processAudio(buffer, numSamples);

        // emit mp3 data
        if (m_mp3Stream)
        {
            m_mp3Stream->writeAudio(buffer, numSamples);

            m_samplesSaved += numSamples;
        }
    }
}

} // namespace Recording
