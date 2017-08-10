#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "recording/coordinator.h"

#include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>

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
