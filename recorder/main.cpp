#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationDomain("Genosse Einhorn");
    QCoreApplication::setApplicationName("KuemmelRecorder");

    MainWindow w;
    w.show();

    return a.exec();
}
