#include "coordinator.h"

#include "levelcalculator.h"
#include "lameencoderstream.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QtMath>

#include <cmath>
#include <cstring>

#include <soundio.h>

namespace
{
    const int SAMPLE_RATE = 48000;
    const int SAMPLE_SIZE = 2 * sizeof(float); // 2*4 bytes

    int soundio_x_ring_buffer_read(SoundIoRingBuffer *buffer, void* out, int bytes)
    {
        int fill_bytes = soundio_ring_buffer_fill_count(buffer);
        char *read_buf = soundio_ring_buffer_read_ptr(buffer);

        int copy_bytes = std::min(bytes, fill_bytes);
        std::memcpy(out, read_buf, copy_bytes);

        soundio_ring_buffer_advance_read_ptr(buffer, copy_bytes);

        return copy_bytes;
    }
}

namespace Recording {

Coordinator::Coordinator(QObject *parent) : QObject(parent)
{
    m_soundio = soundio_create();
    soundio_connect(m_soundio);
    soundio_flush_events(m_soundio);

    m_levelCalculator = new LevelCalculator(this);

    QObject::connect(m_levelCalculator, &LevelCalculator::levelUpdate, this, &Coordinator::handleLevelUpdate);

    m_recordRingBuffer = soundio_ring_buffer_create(m_soundio, SAMPLE_RATE * SAMPLE_SIZE * 10);
    m_monitorRingBuffer = soundio_ring_buffer_create(m_soundio, SAMPLE_RATE * SAMPLE_SIZE * 1);

    QTimer *t = new QTimer(this);
    t->setInterval(40);
    QObject::connect(t, &QTimer::timeout, this, &Coordinator::processAudio);
    t->start();
}

Coordinator::~Coordinator()
{
    stopAudioInput();
    stopMonitorOutput();

    soundio_destroy(m_soundio);
    soundio_ring_buffer_destroy(m_recordRingBuffer);
    soundio_ring_buffer_destroy(m_monitorRingBuffer);
}

bool Coordinator::isSupportedInput(SoundIoDevice *device)
{
    return (device->aim == SoundIoDeviceAimInput)
        && soundio_device_supports_format(device, SoundIoFormatFloat32NE)
        && soundio_device_supports_layout(device, soundio_channel_layout_get_builtin(SoundIoChannelLayoutIdStereo))
        && soundio_device_supports_sample_rate(device, 48000);
}

bool Coordinator::isSupportedOutput(SoundIoDevice *device)
{
    return (device->aim == SoundIoDeviceAimOutput)
        && soundio_device_supports_format(device, SoundIoFormatFloat32NE)
        && soundio_device_supports_layout(device, soundio_channel_layout_get_builtin(SoundIoChannelLayoutIdStereo))
        && soundio_device_supports_sample_rate(device, 48000);
}

void Coordinator::setRecordingDevice(const QString &deviceId)
{
    if (deviceId != m_recordingDevId)
    {
        stopAudioInput();

        m_recordingDevId = deviceId;
        emit recordingDeviceChanged(m_recordingDevId);

        startAudioInput();
    }
}

void Coordinator::setMonitorDevice(const QString &device)
{
    if (device != m_monitorDevId)
    {
        stopMonitorOutput();

        m_monitorDevId = device;
        emit monitorDeviceChanged(m_monitorDevId);

        startMonitorOutput();
    }
}

void Coordinator::setMonitorEnabled(bool enabled)
{
    if (m_monitorEnabled.exchange(enabled) != enabled)
    {
        emit monitorEnabledChanged(m_monitorEnabled);
    }
}

void Coordinator::startRecording()
{
    if (isRecording())
        stopRecording();

    if (!m_audioInStream || !m_recordingDevId.size())
    {
        error(tr("You can't start recoding until your audio input works"));
        stopRecording();
        return;
    }

    QString filename = QString(tr("Recording from %2.mp3"))
            .arg(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd hhmm t")));

    m_mp3FileStream = new QFile(QDir::cleanPath(QString("%1/%2").arg(m_saveDir).arg(filename)));
    if (!m_mp3FileStream->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        error(tr("MP3: Could not open file %1: %2").arg(filename, m_mp3FileStream->errorString()));
        stopRecording();
        return;
    }

    QString track = tr("Recording from %1").arg(QDateTime::currentDateTime().toString(Qt::DefaultLocaleLongDate));

    m_mp3Stream = new LameEncoderStream(this);
    QObject::connect(m_mp3Stream, &LameEncoderStream::error, this, &Coordinator::error);

    if (!m_mp3Stream->init(m_mp3ArtistName, track, 192, m_mp3FileStream))
    {
        stopRecording();
        return;
    }

    emit recordingChanged(isRecording());
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
    m_volumeFactor.store(factor);
    emit volumeFactorChanged(m_volumeFactor);
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

void Coordinator::read_callback(SoundIoInStream *instream, int frame_count_min, int frame_count_max)
{
    Coordinator *c = static_cast<Coordinator*>(instream->userdata);
    struct SoundIoChannelArea *areas;
    int err;

    float multiplier_rec = c->m_volumeFactor.load(std::memory_order_relaxed);
    float multiplier_mon = multiplier_rec;
    if (!c->m_monitorEnabled.load(std::memory_order_relaxed))
        multiplier_mon = 0;

    char *write_ptr_rec = soundio_ring_buffer_write_ptr(c->m_recordRingBuffer);
    char *write_ptr_mon = soundio_ring_buffer_write_ptr(c->m_monitorRingBuffer);
    int free_bytes_rec = soundio_ring_buffer_free_count(c->m_recordRingBuffer);
    int free_bytes_mon = soundio_ring_buffer_free_count(c->m_monitorRingBuffer);
    int free_count_rec = free_bytes_rec / instream->bytes_per_frame;
    int free_count_mon = free_bytes_mon / instream->bytes_per_frame;

    if (free_count_rec < frame_count_min && free_count_mon < frame_count_min)
    {
        // ringbuffer overflow... we'll ignore it and just skip the samples this time
        return;
    }

    int write_frames_rec = std::min(free_count_rec, frame_count_max);
    int write_frames_mon = std::min(free_count_mon, frame_count_max);
    int frames_left_rec = write_frames_rec;
    int frames_left_mon = write_frames_mon;

    for (;;)
    {
        int frame_count = frames_left_rec;

        if ((err = soundio_instream_begin_read(instream, &areas, &frame_count)))
        {
            // FIXME: how to handle this?
            qWarning() << "begin read error: " << soundio_strerror(err);
            return;
        }

        if (!frame_count)
            break;

        if (!areas)
        {
            // Due to an overflow there is a hole. Fill the ring buffer with
            // silence for the size of the hole.
            std::memset(write_ptr_rec, 0, std::max(0, std::min(frame_count, frames_left_rec)) * instream->bytes_per_frame);
            std::memset(write_ptr_mon, 0, std::max(0, std::min(frame_count, frames_left_mon)) * instream->bytes_per_frame);
        }
        else
        {
            for (int frame = 0; frame < frame_count; frame += 1)
            {
                for (int ch = 0; ch < instream->layout.channel_count; ch += 1)
                {
                    Q_ASSERT(sizeof(float) == instream->bytes_per_sample);
                    *((float*)write_ptr_rec) = *((float*)areas[ch].ptr) * multiplier_rec;
                    write_ptr_rec += instream->bytes_per_sample;

                    if (frame < frames_left_mon)
                    {
                        *((float*)write_ptr_mon) = *((float*)areas[ch].ptr) * multiplier_mon;
                        write_ptr_mon += instream->bytes_per_sample;
                    }

                    areas[ch].ptr += areas[ch].step;
                }
            }
        }

        if ((err = soundio_instream_end_read(instream)))
        {
            // FIXME: how to handle this?
            qWarning() << "end read error: " << soundio_strerror(err);
            return;
        }

        frames_left_rec -= frame_count;
        frames_left_mon -= frame_count;
        if (frames_left_rec <= 0)
            break;
    }

    int advance_bytes_rec = write_frames_rec * instream->bytes_per_frame;
    int advance_bytes_mon = write_frames_mon * instream->bytes_per_frame;
    soundio_ring_buffer_advance_write_ptr(c->m_recordRingBuffer, advance_bytes_rec);
    soundio_ring_buffer_advance_write_ptr(c->m_monitorRingBuffer, advance_bytes_mon);
}

void Coordinator::write_callback(SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
    Coordinator *c = static_cast<Coordinator*>(outstream->userdata);
    struct SoundIoChannelArea *areas;
    int frame_count;
    int err;

    char *read_ptr = soundio_ring_buffer_read_ptr(c->m_monitorRingBuffer);
    int fill_bytes = soundio_ring_buffer_fill_count(c->m_monitorRingBuffer);
    int fill_count = fill_bytes / outstream->bytes_per_frame;

    if (frame_count_min > fill_count)
    {
        // Ring buffer does not have enough data, fill with zeroes.
        for (;;) {
            if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count)))
            {
                qWarning() << "begin write error: " << soundio_strerror(err);
                return;
            }
            if (frame_count <= 0)
                return;
            for (int frame = 0; frame < frame_count; frame += 1) {
                for (int ch = 0; ch < outstream->layout.channel_count; ch += 1) {
                    memset(areas[ch].ptr, 0, outstream->bytes_per_sample);
                    areas[ch].ptr += areas[ch].step;
                }
            }
            if ((err = soundio_outstream_end_write(outstream)))
                qWarning() << "end write error: " << soundio_strerror(err);
        }
    }

    int read_count = std::min(frame_count_max, fill_count);
    int frames_left = read_count;

    while (frames_left > 0)
    {
        int frame_count = frames_left;

        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count)))
        {
            qWarning() << "begin write error: " << soundio_strerror(err);
            return;
        }

        if (frame_count <= 0)
            break;

        for (int frame = 0; frame < frame_count; frame += 1)
        {
            for (int ch = 0; ch < outstream->layout.channel_count; ch += 1)
            {
                memcpy(areas[ch].ptr, read_ptr, outstream->bytes_per_sample);
                areas[ch].ptr += areas[ch].step;
                read_ptr += outstream->bytes_per_sample;
            }
        }

        if ((err = soundio_outstream_end_write(outstream)))
        {
            qWarning() << "end write error: " << soundio_strerror(err);
            return;
        }

        frames_left -= frame_count;
    }

    soundio_ring_buffer_advance_read_ptr(c->m_monitorRingBuffer, read_count * outstream->bytes_per_frame);
}

void Coordinator::stopAudioInput()
{
    if (!m_audioInStream)
        return;

    soundio_instream_destroy(m_audioInStream);
    m_audioInStream = nullptr;
}

void Coordinator::startAudioInput()
{
    if (m_audioInStream)
        stopAudioInput();

    if (!m_recordingDevId.size())
        return;

    SoundIoDevice *dev = nullptr;
    int input_dev_count = soundio_input_device_count(m_soundio);
    for (int i = 0; i < input_dev_count; ++i)
    {
        SoundIoDevice *device = soundio_get_input_device(m_soundio, i);

        if (QString(device->id) == m_recordingDevId)
        {
            dev = device;
            break;
        }
        else
        {
            soundio_device_unref(device);
        }
    }
    if (!dev)
    {
        m_recordingDevId = QString();
        return;
    }

    SoundIoInStream *stream = soundio_instream_create(dev);
    stream->format = SoundIoFormatFloat32NE;
    stream->sample_rate = SAMPLE_RATE;
    stream->software_latency = 0.01;
    stream->read_callback = &Coordinator::read_callback;
    stream->overflow_callback = [](SoundIoInStream *) {
        qWarning() << "Audio input overflow!";
    };
    stream->userdata = static_cast<void*>(this);

    int err = soundio_instream_open(stream);
    soundio_device_unref(dev);

    if (err)
    {
        emit error(soundio_strerror(err));
        soundio_instream_destroy(stream);
        return;
    }

    err = soundio_instream_start(stream);

    if (err)
    {
        emit error(soundio_strerror(err));
        soundio_instream_destroy(stream);
        return;
    }
    else
    {
        emit error(QString());
    }

    m_audioInStream = stream;
}

void Coordinator::stopMonitorOutput()
{
    if (!m_audioOutStream)
        return;

    soundio_outstream_destroy(m_audioOutStream);
    m_audioOutStream = nullptr;
}

void Coordinator::startMonitorOutput()
{
    if (m_audioOutStream)
        stopMonitorOutput();

    if (!m_monitorDevId.size())
        return;

    SoundIoDevice *dev = nullptr;
    int output_dev_count = soundio_output_device_count(m_soundio);
    for (int i = 0; i < output_dev_count; ++i)
    {
        SoundIoDevice *device = soundio_get_output_device(m_soundio, i);

        if (QString(device->id) == m_monitorDevId)
        {
            dev = device;
            break;
        }
        else
        {
            soundio_device_unref(device);
        }
    }
    if (!dev)
    {
        m_monitorDevId = QString();
        return;
    }

    SoundIoOutStream *stream = soundio_outstream_create(dev);
    stream->format = SoundIoFormatFloat32NE;
    stream->sample_rate = SAMPLE_RATE;
    stream->software_latency = 0.01;
    stream->write_callback = &Coordinator::write_callback;
    stream->underflow_callback = [](SoundIoOutStream *) {
        qWarning() << "Audio output underflow!";
    };
    stream->userdata = static_cast<void*>(this);

    int err = soundio_outstream_open(stream);
    soundio_device_unref(dev);

    if (err)
    {
        emit error(soundio_strerror(err));
        soundio_outstream_destroy(stream);
        return;
    }

    err = soundio_outstream_start(stream);

    if (err)
    {
        emit error(soundio_strerror(err));
        soundio_outstream_destroy(stream);
        return;
    }
    else
    {
        emit error(QString());
    }

    m_audioOutStream = stream;
}

void Coordinator::processAudio()
{
    float buffer[2048];
    int nbytes = 0;

    while ((nbytes = soundio_x_ring_buffer_read(m_recordRingBuffer, buffer, sizeof(buffer))))
    {
        int nsamples = nbytes / SAMPLE_SIZE;

        // do level calculation
        m_levelCalculator->processAudio(buffer, nsamples);

        if (m_mp3Stream)
        {
            m_mp3Stream->writeAudio(buffer, nsamples);

            m_samplesSaved += nsamples;
        }
    }
}

} // namespace Recording