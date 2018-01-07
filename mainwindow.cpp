#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "recording/coordinator.h"

#include <lame/lame.h>

#include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_recorder = new Recording::Coordinator(this);
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

    for (QWidget *w: this->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
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
    this->setPalette(windowPalette);

    QObject::connect(ui->bAbout, &QAbstractButton::clicked, this, &MainWindow::showAboutDialog);
}

MainWindow::~MainWindow()
{
    delete ui;
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
        "Copyright &copy; 2017 Jonas Kümmerlin &lt;jonas@kuemmerlin.eu&gt;"
        "<p>"
        "Running on <br>"
        "<a href=\"https://www.qt.io/\">Qt</a> %%QT_VERSION%%<br>"
        "<a href=\"http://lame.sourceforge.net/\">LAME</a> %%LAME_VERSION%%<br>"
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
        .replace("%%LAME_VERSION%%", get_lame_version()));
}
