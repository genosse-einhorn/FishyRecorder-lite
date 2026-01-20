#include "levelcalculator.h"

namespace Recording {

LevelCalculator::LevelCalculator(QObject *parent) : QObject(parent)
{

}

void LevelCalculator::processAudio(const float *samples, qint64 count)
{
    for (qint64 i = 0; i < count; ++i) {
        m_tempLevelL = qMax(m_tempLevelL, qAbs(samples[2*i + 0]));
        m_tempLevelR = qMax(m_tempLevelR, qAbs(samples[2*i + 1]));
    }

    m_tempLevelSampleCount += count;

    if (m_tempLevelSampleCount >= 2000) {
        m_levelL = m_tempLevelL;
        m_levelR = m_tempLevelR;
        m_tempLevelSampleCount = 0;
        m_tempLevelL = 0;
        m_tempLevelR = 0;

        emit levelUpdate(m_levelL, m_levelR);
    }
}

} // namespace Recording
