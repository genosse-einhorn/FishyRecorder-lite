#ifndef LEVELCALCULATOR_H
#define LEVELCALCULATOR_H

#include <QObject>

class RecordingLevelCalculator : public QObject
{
    Q_OBJECT
public:
    explicit RecordingLevelCalculator(QObject *parent = 0);

signals:
    void levelUpdate(float l, float r);

public slots:
    void processAudio(const float *samples, qint64 count);
    void processAudio(const qint16 *samples, qint64 count);

private:
    float m_tempLevelL = 0.0f;
    float m_tempLevelR = 0.0f;
    float m_levelL = 0.0f;
    float m_levelR = 0.0f;
    qint64 m_tempLevelSampleCount = 0;
};

#endif // LEVELCALCULATOR_H
