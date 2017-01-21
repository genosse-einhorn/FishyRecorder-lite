#include "recordingconfiguratorpane.h"
#include "ui_recordingconfiguratorpane.h"

#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <portaudio.h>

#include "recordingcoordinator.h"

RecordingConfiguratorPane::RecordingConfiguratorPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordingConfiguratorPane)
{
    Pa_Initialize();

    ui->setupUi(this);

    QSettings settings;

    ui->cbRecordDev->addItem(tr("<No Device>"), QVariant::fromValue(paNoDevice));
    ui->cbMonitorDev->addItem(tr("<No Device>"), QVariant::fromValue(paNoDevice));
    for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);

        if (RecordingCoordinator::isSupportedInput(i, info))
        {
            ui->cbRecordDev->addItem(QString::fromLocal8Bit(info->name), QVariant::fromValue(i));
        }

        if (RecordingCoordinator::isSupportedOutput(i, info))
        {
            ui->cbMonitorDev->addItem(QString::fromLocal8Bit(info->name), QVariant::fromValue(i));
        }
    }

    auto i = ui->cbRecordDev->findText(settings.value("Recording Device", QVariant::fromValue(QString())).toString());
    if (i >= 0)
    {
        ui->cbRecordDev->setCurrentIndex(i);
    }
    else
    {
        ui->cbRecordDev->setCurrentIndex(ui->cbRecordDev->findData(QVariant::fromValue(Pa_GetDefaultInputDevice())));
    }

    i = ui->cbMonitorDev->findText(settings.value("Monitor Device", QVariant::fromValue(QString())).toString());
    if (i >= 0)
    {
        ui->cbMonitorDev->setCurrentIndex(i);
    }
    else
    {
        ui->cbMonitorDev->setCurrentIndex(ui->cbMonitorDev->findData(QVariant::fromValue(Pa_GetDefaultOutputDevice())));
    }

    ui->slVolume->setValue(settings.value("Volume", QVariant::fromValue(100000000)).toInt());

    ui->eDirectory->setText(settings.value("Output Directory",
        QVariant::fromValue(QStandardPaths::writableLocation(QStandardPaths::MusicLocation))).toString());

    QString artist = settings.value("MP3 Artist").toString();
    if (!artist.size())
        artist = QString::fromUtf8(qgetenv("USER"));
    if (!artist.size())
        artist = QString::fromUtf8(qgetenv("USERNAME"));
    if (!artist.size())
        artist = tr("Someone");
    ui->eMp3Artist->setText(artist);



    QObject::connect(ui->cbMonitorDev, &QComboBox::currentTextChanged, this, &RecordingConfiguratorPane::cbMonitorDevChanged);
    QObject::connect(ui->cbRecordDev, &QComboBox::currentTextChanged, this, &RecordingConfiguratorPane::cbRecordDevChanged);
    QObject::connect(ui->slVolume, &QSlider::valueChanged, this, &RecordingConfiguratorPane::slVolumeChanged);
    QObject::connect(ui->bPicker, &QAbstractButton::clicked, this, &RecordingConfiguratorPane::outputDirButtonClick);
    QObject::connect(ui->eMp3Artist, &QLineEdit::textChanged, this, &RecordingConfiguratorPane::eMp3ArtistTextChanged);
}

RecordingConfiguratorPane::~RecordingConfiguratorPane()
{
    delete ui;

    Pa_Terminate();
}

void RecordingConfiguratorPane::hookupCoordinator(RecordingCoordinator *c)
{
    QObject::connect(this, &RecordingConfiguratorPane::monitorDevChanged, c, &RecordingCoordinator::setMonitorDevice);
    QObject::connect(this, &RecordingConfiguratorPane::recordingDevChanged, c, &RecordingCoordinator::setRecordingDevice);
    QObject::connect(this, &RecordingConfiguratorPane::volumeChanged, c, &RecordingCoordinator::setVolumeFactor);
    QObject::connect(this, &RecordingConfiguratorPane::outputDirChanged, c, &RecordingCoordinator::setSaveDir);
    QObject::connect(this, &RecordingConfiguratorPane::mp3ArtistChanged, c, &RecordingCoordinator::setMp3ArtistName);

    // initial sync
    cbRecordDevChanged();
    cbMonitorDevChanged();
    slVolumeChanged();
    eMp3ArtistTextChanged();
    emit outputDirChanged(ui->eDirectory->text());
}

void RecordingConfiguratorPane::cbRecordDevChanged()
{
    QSettings().setValue("Recording Device", QVariant::fromValue(ui->cbRecordDev->currentText()));
    emit recordingDevChanged(ui->cbRecordDev->currentData().value<PaDeviceIndex>());
}

void RecordingConfiguratorPane::cbMonitorDevChanged()
{
    QSettings().setValue("Monitor Device", QVariant::fromValue(ui->cbMonitorDev->currentText()));
    emit monitorDevChanged(ui->cbMonitorDev->currentData().value<PaDeviceIndex>());
}

void RecordingConfiguratorPane::slVolumeChanged()
{
    QSettings().setValue("Volume", QVariant::fromValue(ui->slVolume->value()));
    double val = double(ui->slVolume->value()) / 100000000.0;
    emit volumeChanged(val);
}

void RecordingConfiguratorPane::outputDirButtonClick()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->eDirectory->text());
    if (dir.length())
    {
        ui->eDirectory->setText(dir);

        QSettings().setValue("Output Directory", QVariant::fromValue(dir));

        emit outputDirChanged(dir);
    }
}

void RecordingConfiguratorPane::eMp3ArtistTextChanged()
{
    QSettings().setValue("MP3 Artist", QVariant::fromValue(ui->eMp3Artist->text()));
    emit mp3ArtistChanged(ui->eMp3Artist->text());
}
