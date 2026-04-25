#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>

namespace Logger {
    void install();
    void shutdown();
    QString currentLogFile();
    QString alsoftLogFile();
} // namespace Logger

#endif // LOGGER_H
