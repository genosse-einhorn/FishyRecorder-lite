#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationDomain("Genosse Einhorn");
    QCoreApplication::setApplicationName("KuemmelRecorder");

    QTranslator t;
    t.load(QLocale::system(), ":/l10n/");
    a.installTranslator(&t);

    MainWindow w;
    w.show();

    return a.exec();
}
