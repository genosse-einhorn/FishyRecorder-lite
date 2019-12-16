#include "flacencoderstream.h"

#include <cmath>
#include <QIODevice>
#include <QDebug>

namespace {

FLAC__StreamEncoderWriteStatus writeToQIODevice(const FLAC__StreamEncoder *encoder,
                                                const FLAC__byte buffer[],
                                                size_t bytes,
                                                unsigned samples,
                                                unsigned current_frame,
                                                void *client_data)
{
    Q_UNUSED(samples);
    Q_UNUSED(current_frame);
    Q_UNUSED(encoder);

    QIODevice *dev = (QIODevice*)client_data;

    if (dev->write((char*)buffer, bytes) == (qint64)bytes)
        return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    else
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}

} // namespace

namespace Recording {

FlacEncoderStream::FlacEncoderStream(QObject *parent) : AbstractEncoderStream(parent)
{
    m_encoder = FLAC__stream_encoder_new();
    FLAC__stream_encoder_set_streamable_subset(m_encoder, true);
    FLAC__stream_encoder_set_channels(m_encoder, 2);
    FLAC__stream_encoder_set_bits_per_sample(m_encoder, 16);
    FLAC__stream_encoder_set_compression_level(m_encoder, 7);
}

FlacEncoderStream::~FlacEncoderStream()
{
    close();
    FLAC__stream_encoder_delete(m_encoder);
    m_encoder = nullptr;

    if (m_vorbiscomm) FLAC__metadata_object_delete(m_vorbiscomm);
    m_vorbiscomm = nullptr;
}

} // namespace Recording


bool Recording::FlacEncoderStream::init(const QString &artist, const QString &track, int samplerate, QIODevice *output)
{
    // FIXME!
    Q_UNUSED(artist);
    Q_UNUSED(track);

    FLAC__stream_encoder_set_sample_rate(m_encoder, samplerate);

    if (m_vorbiscomm) FLAC__metadata_object_delete(m_vorbiscomm);
    m_vorbiscomm = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    if (artist.size())
    {
        QByteArray c = QStringLiteral("ARTIST=%1").arg(artist).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(m_vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    if (track.size())
    {
        QByteArray c = QStringLiteral("TITLE=%1").arg(track).toUtf8();
        FLAC__metadata_object_vorbiscomment_append_comment(m_vorbiscomm, { (FLAC__uint32)c.size(), (FLAC__byte*)c.data() }, true);
    }

    FLAC__stream_encoder_set_metadata(m_encoder, &m_vorbiscomm, 1u);

    auto status = FLAC__stream_encoder_init_stream(m_encoder, &writeToQIODevice, nullptr, nullptr, nullptr, output);

    if (status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
    {
        emit error(tr("FLAC Error: %1").arg(FLAC__StreamEncoderInitStatusString[status]));
        qCritical() << "FLAC Error: " << FLAC__StreamEncoderInitStatusString[status];
        return false;
    }

    return true;
}

void Recording::FlacEncoderStream::close()
{
    if (m_encoder)
        FLAC__stream_encoder_finish(m_encoder);
}

qint64 Recording::FlacEncoderStream::writeAudio(float *samples, qint64 count)
{
    for (qint64 i = 0; i < count; ++i)
    {
        float sampleL = samples[2*i];
        float sampleR = samples[2*i + 1];

        float aL = (sampleL * 32768.0f) + m_errL + m_randomDistribution(m_randomGenerator);
        float aR = (sampleR * 32768.0f) + m_errR + m_randomDistribution(m_randomGenerator);

        float qL = qBound(-32768.0f, std::floor(aL), 32767.0f);
        float qR = qBound(-32768.0f, std::floor(aR), 32767.0f);

        m_errL = aL - qL;
        m_errR = aR - qR;

        int32_t samples[] = { int32_t(qL), int32_t(qR) };
        if (!FLAC__stream_encoder_process_interleaved(m_encoder, samples, 1))
        {
            auto status = FLAC__stream_encoder_get_state(m_encoder);
            emit error(tr("FLAC Error: %1").arg(FLAC__StreamEncoderInitStatusString[status]));
            qCritical() << "FLAC Error: " << FLAC__StreamEncoderInitStatusString[status];
            return i;
        }
    }

    return count;
}
