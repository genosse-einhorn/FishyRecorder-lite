#include "logger.h"

#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QDebug>
#include <QTemporaryFile>
#include <cstdio>

namespace {

QString logfileDir(void)
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

struct LoggerImpl
{
    QMutex m_mutex;
    QTemporaryFile m_logfile;

    bool init(void)
    {
        QMutexLocker locker(&m_mutex);

        auto dir = logfileDir();
        auto tmpl = QString("%1/%2.%3.XXXXXX.log").arg(dir,
                                                   QApplication::applicationName(),
                                                   QDateTime::currentDateTime().toString("yyyyMMdd.HHmmss"));
        if (!QDir::root().mkpath(dir)) {
            qWarning() << "couldn't create directory" << dir;
        }

        m_logfile.setFileTemplate(tmpl);
        m_logfile.setAutoRemove(false);
        if (!m_logfile.open()) {
            qWarning() << "couldn't open file" << m_logfile.fileName() << ": " << m_logfile.errorString();
            return false;
        }
        m_logfile.setTextModeEnabled(true);

        return true;
    }

    void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QLatin1String typestr;
        switch (type) {
        case QtDebugMsg:
            typestr = QLatin1String("DEBUG");
            break;
        case QtInfoMsg:
            typestr = QLatin1String("INFO");
            break;
        case QtWarningMsg:
            typestr = QLatin1String("WARNING");
            break;
        case QtCriticalMsg:
            typestr = QLatin1String("CRITICAL");
            break;
        case QtFatalMsg:
            typestr = QLatin1String("FATAL");
            break;
        }

        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";

        QString outstr = QString("%1:\t%2\t%6 (%3:%5, %4)\n")
                .arg(typestr)
                .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
                .arg(file)
                .arg(function)
                .arg(context.line)
                .arg(msg);

        QByteArray outstrUtf8 = outstr.toUtf8();

        {
            QMutexLocker locker(&m_mutex);
            if (m_logfile.isOpen()) {
                m_logfile.write(outstrUtf8);
                m_logfile.flush();
            }
        }

        std::fprintf(stderr, "%s", outstrUtf8.constData());
    }
};

static LoggerImpl *impl = nullptr;

void myLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    impl->logHandler(type, context, msg);
}

const qint64 ALWAYS_KEEP_FILE_NO = 5; // always keep the last 5 log files
const qint64 TOTAL_SIZE_LIMIT = 500000; // limit logs to 500kb of disk space

void cleanupOldLogFiles(void)
{
    auto dir = QDir(logfileDir());

    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList() << QString("%1.*.log").arg(QApplication::applicationName()));
    dir.setSorting(QDir::Time);

    qint64 counter = 0;
    qint64 sizesum = 0;
    for (QFileInfo i : dir.entryInfoList()) {
        counter++;
        sizesum += i.size();

        if (counter <= ALWAYS_KEEP_FILE_NO)
            continue;

        if (sizesum > TOTAL_SIZE_LIMIT) {
            qInfo() << "removing log file" << i.absoluteFilePath();
            QFile::remove(i.absoluteFilePath());
        }
    }
}

} // namespace

namespace Logger {

void install()
{
    if (impl)
        return;

    impl = new LoggerImpl();

    if (impl->init()) {
        qInstallMessageHandler(myLogHandler);

        qInfo() << "Beginning to log to file" << impl->m_logfile.fileName();

        cleanupOldLogFiles();
    }
}

void shutdown()
{
    if (!impl)
        return;

    qInstallMessageHandler(nullptr);

    delete impl;
    impl = nullptr;
}

QString currentLogFile()
{
    if (!impl)
        return QString();

    return impl->m_logfile.fileName();
}



} // namespace Logger
