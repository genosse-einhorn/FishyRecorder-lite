#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationDomain("Genosse Einhorn");
    QCoreApplication::setApplicationName("KuemmelRecorder");

    Logger::install();

    QTranslator t;
    t.load(QLocale::system(), ":/l10n/");
    a.installTranslator(&t);

    MainWindow w;
    w.show();

    int rv = a.exec();

    Logger::shutdown();

    return rv;
}
