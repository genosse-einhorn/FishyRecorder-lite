#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "recordingcoordinator.h"

#include <portaudio.h>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_recorder = new RecordingCoordinator(this);
    QObject::connect(m_recorder, &RecordingCoordinator::statusUpdate, ui->statusView, &RecordingStatusView::handleStatusUpdate);
    QObject::connect(m_recorder, &RecordingCoordinator::error, ui->errorWidget, &RecordingErrorWidget::displayError);

    QObject::connect(ui->bEnableMonitor, &QAbstractButton::toggled, m_recorder, &RecordingCoordinator::setMonitorEnabled);
    QObject::connect(ui->bEnableRecord, &QAbstractButton::toggled, this, &MainWindow::recordToggle);

    ui->configPane->hookupCoordinator(m_recorder);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::recordToggle(bool record)
{
    if (record)
    {
        m_recorder->startRecording();
    }
    else
    {
        m_recorder->stopRecording();
    }
}
