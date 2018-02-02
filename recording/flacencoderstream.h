#ifndef FLACENCODERSTREAM_H
#define FLACENCODERSTREAM_H

#include "abstractencoderstream.h"

#include "FLAC/stream_encoder.h"
#include <random>

namespace Recording {

class FlacEncoderStream : public AbstractEncoderStream
{
public:
    explicit FlacEncoderStream(QObject *parent = nullptr);
    ~FlacEncoderStream() override;

public:
    bool init(const QString &artist, const QString &track, int samplerate, QIODevice *output) override;
    void close() override;
    qint64 writeAudio(float *samples, qint64 count) override;

private:
    FLAC__StreamEncoder *m_encoder { nullptr };

    // dither state
    std::mt19937 m_randomGenerator { std::random_device()() };
    std::uniform_real_distribution<float> m_randomDistribution { -1.0f, 1.0f };
    float m_errL { 0.0f };
    float m_errR { 0.0f };
};

} // namespace Recording

#endif // FLACENCODERSTREAM_H
