#ifndef LAMEENCODERSTREAM_H
#define LAMEENCODERSTREAM_H

#include "abstractencoderstream.h"

#include <lame/lame.h>

class QIODevice;

namespace Recording {

class LameEncoderStream: public AbstractEncoderStream
{
    Q_OBJECT
public:
    explicit LameEncoderStream(int brate, QObject *parent = nullptr);
    ~LameEncoderStream();

    bool init(const QString& artist, const QString &track, int samplerate, QIODevice *output) override;
    qint64 writeAudio(float *samples, qint64 count) override;
    void close() override;

signals:
    void error(const QString &message);

private:
    lame_global_flags *m_lame_gbf;
    QIODevice         *m_device;
};

} // namespace Recording

#endif // LAMEENCODERSTREAM_H
