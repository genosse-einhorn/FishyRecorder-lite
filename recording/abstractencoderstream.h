#ifndef ABSTRACTENCODERSTREAM_H
#define ABSTRACTENCODERSTREAM_H

#include <QObject>

class QIODevice;

namespace Recording {

class AbstractEncoderStream : public QObject
{
    Q_OBJECT
public:
    explicit AbstractEncoderStream(QObject *parent = nullptr);

    virtual bool init(const QString& artist, const QString &track, int samplerate, QIODevice *output) = 0;
    virtual void close() = 0;
    virtual qint64 writeAudio(float *samples, qint64 count) = 0;

signals:
    void error(const QString &message);
};

} // namespace Recording

#endif // ABSTRACTENCODERSTREAM_H
