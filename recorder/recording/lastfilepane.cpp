#include "lastfilepane.h"
#include "ui_lastfilepane.h"

#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QProcess>

namespace Recording {

LastFilePane::LastFilePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LastFilePane)
{
    ui->setupUi(this);
    QSizePolicy p = this->sizePolicy();
    p.setRetainSizeWhenHidden(true);
    this->setSizePolicy(p);
    this->setVisible(false);

    connect(ui->showButton, &QAbstractButton::clicked, this, &LastFilePane::showFileInExplorer);
    connect(ui->label, &QLabel::linkActivated, this, &LastFilePane::showFileInExplorer);
}

LastFilePane::~LastFilePane()
{
    delete ui;
}

void LastFilePane::newRecordingFile(const QString &file)
{
    m_file = file;

    ui->label->setText(tr("Last written to file <a href=\"#\">%1</a>").arg(QFileInfo(file).fileName()));

    this->setVisible(true);
}

void LastFilePane::showFileInExplorer()
{
#if defined(Q_OS_WIN32)
    QString filename = QDir::toNativeSeparators(m_file);
    QProcess explorer;
    explorer.setProgram("explorer.exe");
    explorer.setNativeArguments(QString("/select,\"%1\"").arg(filename));
    explorer.start();
    explorer.waitForFinished();

#elif defined(Q_OS_UNIX)
    auto url = QUrl::fromLocalFile(m_file).toString(QUrl::FullyEncoded);
    QProcess::execute("dbus-send", {
        "--print-reply",
        "--dest=org.freedesktop.FileManager1",
        "/org/freedesktop/FileManager1",
        "org.freedesktop.FileManager1.ShowItems",
        QString("array:string:%1").arg(url),
        "string:"
    });
#endif
}

} // namespace Recording
