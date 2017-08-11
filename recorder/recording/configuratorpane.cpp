#include "configuratorpane.h"
#include "ui_recordingconfiguratorpane.h"

#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

#include <soundio.h>

#include "coordinator.h"

namespace {
    QString getDeviceDesc(SoundIoDevice *dev)
    {
        if (dev->is_raw)
            return QString("%1 [RAW]").arg(QString::fromUtf8(dev->name));
        else
            return QString("%1").arg(QString::fromUtf8(dev->name));
    }
}

namespace Recording {

ConfiguratorPane::ConfiguratorPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecordingConfiguratorPane)
{
    ui->setupUi(this);

    SoundIo *soundio = soundio_create();
    soundio_connect(soundio);
    soundio_flush_events(soundio);

    QSettings settings;

    ui->cbRecordDev->addItem(tr("<No Device>"), QVariant::fromValue(QString()));
    ui->cbMonitorDev->addItem(tr("<No Device>"), QVariant::fromValue(QString()));

    int input_count = soundio_input_device_count(soundio);
    int output_count = soundio_output_device_count(soundio);

    for (int i = 0; i < input_count; ++i)
    {
        SoundIoDevice *device = soundio_get_input_device(soundio, i);

        if (Coordinator::isSupportedInput(device))
        {
            ui->cbRecordDev->addItem(getDeviceDesc(device), Coordinator::uniqueDeviceId(device));
        }

        soundio_device_unref(device);
    }

    for (int i = 0; i < output_count; ++i)
    {
        SoundIoDevice *device = soundio_get_output_device(soundio, i);

        if (Coordinator::isSupportedOutput(device))
        {
            ui->cbMonitorDev->addItem(getDeviceDesc(device), Coordinator::uniqueDeviceId(device));
        }

        soundio_device_unref(device);
    }

    auto i = ui->cbRecordDev->findData(settings.value("Recording Device", QVariant::fromValue(QString())));
    if (i >= 0)
    {
        ui->cbRecordDev->setCurrentIndex(i);
    }
    else
    {
        ui->cbRecordDev->setCurrentIndex(ui->cbRecordDev->findData(QVariant::fromValue(QString())));
    }

    i = ui->cbMonitorDev->findData(settings.value("Monitor Device", QVariant::fromValue(QString())));
    if (i >= 0)
    {
        ui->cbMonitorDev->setCurrentIndex(i);
    }
    else
    {
        ui->cbMonitorDev->setCurrentIndex(ui->cbMonitorDev->findData(QVariant::fromValue(QString())));
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

    soundio_destroy(soundio);

    QObject::connect(ui->cbMonitorDev, &QComboBox::currentTextChanged, this, &ConfiguratorPane::cbMonitorDevChanged);
    QObject::connect(ui->cbRecordDev, &QComboBox::currentTextChanged, this, &ConfiguratorPane::cbRecordDevChanged);
    QObject::connect(ui->slVolume, &QSlider::valueChanged, this, &ConfiguratorPane::slVolumeChanged);
    QObject::connect(ui->bPicker, &QAbstractButton::clicked, this, &ConfiguratorPane::outputDirButtonClick);
    QObject::connect(ui->eMp3Artist, &QLineEdit::textChanged, this, &ConfiguratorPane::eMp3ArtistTextChanged);
    QObject::connect(ui->lLastFile, &QLabel::linkActivated, this, &ConfiguratorPane::recordingFileClicked);

    QSizePolicy p = ui->gbOutputFile->sizePolicy();
    p.setRetainSizeWhenHidden(true);
    ui->gbOutputFile->setSizePolicy(p);
    ui->gbOutputFile->setVisible(false);
}

ConfiguratorPane::~ConfiguratorPane()
{
    delete ui;
}

void ConfiguratorPane::hookupCoordinator(Coordinator *c)
{
    QObject::connect(this, &ConfiguratorPane::monitorDevChanged, c, &Coordinator::setMonitorDevice);
    QObject::connect(this, &ConfiguratorPane::recordingDevChanged, c, &Coordinator::setRecordingDevice);
    QObject::connect(this, &ConfiguratorPane::volumeChanged, c, &Coordinator::setVolumeFactor);
    QObject::connect(this, &ConfiguratorPane::outputDirChanged, c, &Coordinator::setSaveDir);
    QObject::connect(this, &ConfiguratorPane::mp3ArtistChanged, c, &Coordinator::setMp3ArtistName);
    QObject::connect(c, &Coordinator::recordingFileOpened, this, &ConfiguratorPane::recordingFileOpened);

    // initial sync
    cbRecordDevChanged();
    cbMonitorDevChanged();
    slVolumeChanged();
    eMp3ArtistTextChanged();
    emit outputDirChanged(ui->eDirectory->text());
}

void ConfiguratorPane::cbRecordDevChanged()
{
    QSettings().setValue("Recording Device", ui->cbRecordDev->currentData());
    emit recordingDevChanged(ui->cbRecordDev->currentData().toString());
}

void ConfiguratorPane::cbMonitorDevChanged()
{
    QSettings().setValue("Monitor Device", ui->cbMonitorDev->currentData());
    emit monitorDevChanged(ui->cbMonitorDev->currentData().toString());
}

void ConfiguratorPane::slVolumeChanged()
{
    QSettings().setValue("Volume", QVariant::fromValue(ui->slVolume->value()));
    double val = double(ui->slVolume->value()) / 100000000.0;
    emit volumeChanged(val);
}

void ConfiguratorPane::outputDirButtonClick()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->eDirectory->text());
    if (dir.length())
    {
        ui->eDirectory->setText(dir);

        QSettings().setValue("Output Directory", QVariant::fromValue(dir));

        emit outputDirChanged(dir);
    }
}

void ConfiguratorPane::eMp3ArtistTextChanged()
{
    QSettings().setValue("MP3 Artist", QVariant::fromValue(ui->eMp3Artist->text()));
    emit mp3ArtistChanged(ui->eMp3Artist->text());
}

void ConfiguratorPane::recordingFileOpened(const QString &file)
{
    ui->gbOutputFile->setVisible(true);
    QString filename = QFileInfo(file).fileName();
    ui->lLastFile->setText(QString("<a href=\"%2\">%1</a>").arg(filename)
                           .arg(QUrl::fromLocalFile(file).toString(QUrl::FullyEncoded)));
}

void ConfiguratorPane::recordingFileClicked(const QString &url)
{
#if defined(Q_OS_WIN32)
    QString filename = QDir::toNativeSeparators(QUrl(url).toLocalFile());
    QProcess explorer;
    explorer.setProgram("explorer.exe");
    explorer.setNativeArguments(QString("/select,\"%1\"").arg(filename));
    explorer.start();
    explorer.waitForFinished();

#elif defined(Q_OS_UNIX)
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
