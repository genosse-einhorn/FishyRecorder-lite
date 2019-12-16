#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "recording/coordinator.h"
#include "recording/utilopenfile.h"
#include "logger.h"

#ifdef HAVE_THIRDPARTY_LICENSES
#   include "thirdpartylicensedialog.h"
#endif

#include <lame/lame.h>
#include <FLAC/format.h>

#include <QDebug>
#include <QProcess>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QMenu>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_recorderThread = new QThread(this);
    m_recorder = new Recording::Coordinator();
    m_recorder->moveToThread(m_recorderThread);
    connect(m_recorderThread, &QThread::finished, m_recorder, &QObject::deleteLater);
    m_recorderThread->start();

    QObject::connect(m_recorder, &Recording::Coordinator::statusUpdate, ui->statusView, &Recording::StatusView::handleStatusUpdate);
    QObject::connect(m_recorder, &Recording::Coordinator::error, ui->errorWidget, &Recording::ErrorWidget::displayError);

    QObject::connect(ui->bEnableMonitor, &QAbstractButton::toggled, m_recorder, &Recording::Coordinator::setMonitorEnabled);
    QObject::connect(ui->bEnableRecord, &QAbstractButton::clicked, m_recorder, &Recording::Coordinator::startRecording);
    QObject::connect(ui->bStop, &QAbstractButton::clicked, m_recorder, &Recording::Coordinator::stopRecording);
    QObject::connect(ui->bNewTrack, &QAbstractButton::clicked, m_recorder, &Recording::Coordinator::startNewTrack);
    QObject::connect(m_recorder, &Recording::Coordinator::recordingChanged, this, &MainWindow::recordingStateChanged);
    QObject::connect(m_recorder, &Recording::Coordinator::monitorEnabledChanged, ui->bEnableMonitor, &QAbstractButton::setChecked);
    QObject::connect(m_recorder, &Recording::Coordinator::recordingFileOpened, ui->lastFilePane, &Recording::LastFilePane::newRecordingFile);

    ui->configPane->hookupCoordinator(m_recorder);

    for (QWidget *w: this->centralWidget()->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
    {
        QPalette p = w->palette();
        p.setColor(QPalette::Background, this->palette().color(QPalette::Background));
        w->setPalette(p);
    }

    QPalette statusPalette = ui->statusView->palette();
    statusPalette.setColor(QPalette::Background, QColor(0x40, 0x42, 0x44));
    statusPalette.setColor(QPalette::Foreground, QColor(0xbf, 0xc1, 0xc2));
    statusPalette.setColor(QPalette::Highlight, QColor(0xba, 0x60, 0x00));
    ui->statusView->setPalette(statusPalette);
    QPalette windowPalette = this->palette();
    windowPalette.setColor(QPalette::Background, statusPalette.color(QPalette::Background));
    this->centralWidget()->setPalette(windowPalette);

    QObject::connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    QObject::connect(ui->actionAudioDebugInfo, &QAction::triggered, this, &MainWindow::showAlDebugDialog);
    QObject::connect(ui->actionLogFiles, &QAction::triggered, this, &MainWindow::showLogFiles);

    // Move buttons into a toolbar and create "Help" menu
    QToolBar *tb = new QToolBar(this);
    tb->setMovable(false);
    tb->toggleViewAction()->setEnabled(false);
    tb->addWidget(ui->bEnableRecord);
    tb->addWidget(ui->bStop);
    tb->addSeparator();
    tb->addWidget(ui->bNewTrack);
    tb->addSeparator();
    tb->addWidget(ui->bEnableMonitor);
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    tb->addWidget(spacer);
    QMenu *helpMenu = new QMenu(this);
    helpMenu->addAction(ui->actionAudioDebugInfo);
    helpMenu->addAction(ui->actionLogFiles);
    helpMenu->addSeparator();

#ifdef HAVE_THIRDPARTY_LICENSES
    QAction *tpLicensesDialog = new QAction(tr("3rd-Party Licenses"), this);
    QObject::connect(tpLicensesDialog, &QAction::triggered, this, [=](){
        ThirdPartyLicenseDialog d(this);
        d.exec();
    });
    helpMenu->addAction(tpLicensesDialog);
    helpMenu->addSeparator();
#endif

    helpMenu->addAction(ui->actionAbout);
    ui->bHelp->setMenu(helpMenu);
    ui->bHelp->setStyleSheet("*:menu-indicator { image: none; }");
    tb->addWidget(ui->bHelp);
    this->addToolBar(Qt::BottomToolBarArea, tb);
    delete ui->buttonContainer;
    ui->buttonContainer = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    m_recorderThread->quit();
    m_recorderThread->wait();
}

void MainWindow::recordingStateChanged(bool isRecording)
{
    ui->bEnableRecord->setEnabled(!isRecording);
    ui->bNewTrack->setEnabled(isRecording);
    ui->bStop->setEnabled(isRecording);
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, tr("About"), tr(
        "<h3>&lt;&gt;&lt; Recorder (Lite)</h3>"
        "%%RECORDER_VERSION%%<br>"
        "Copyright &copy; 2018-2020 Jonas Kümmerlin &lt;jonas@kuemmerlin.eu&gt;"
        "<p>"
        "Running on <br>"
        "<a href=\"https://www.qt.io/\">Qt</a> %%QT_VERSION%%<br>"
        "<a href=\"http://lame.sourceforge.net/\">LAME</a> %%LAME_VERSION%%<br>"
        "<a href=\"https://xiph.org/flac/\">FLAC</a> %%FLAC_VERSION%%<br>"
        "<a href=\"https://www.openal.org/\">OpenAL</a> (<a href=\"http://kcat.strangesoft.net/openal.html\">OpenAL Soft</a>)<br>"
        "<p>"
        "This program is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 2 of the License, or "
        "(at your option) any later version. "
        "<p>"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details."
        "<p>"
        "A copy of the GNU General Public License can be retrieved from "
        "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
     ).replace("%%RECORDER_VERSION%%", GIT_REVNO)
        .replace("%%QT_VERSION%%", qVersion())
        .replace("%%LAME_VERSION%%", get_lame_version())
        .replace("%%FLAC_VERSION%%", FLAC__VERSION_STRING));
}

void MainWindow::showAlDebugDialog()
{
    QMessageBox mb(QMessageBox::NoIcon, tr("OpenAL debug information"), Recording::Coordinator::backendDebugInfo(), QMessageBox::Ok, this);
    mb.setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    mb.exec();
}

void MainWindow::showLogFiles()
{
    Recording::Util::showFileInExplorer(Logger::currentLogFile());
}
