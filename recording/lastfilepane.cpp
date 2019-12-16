#include "lastfilepane.h"
#include "ui_lastfilepane.h"
#include "utilopenfile.h"

#include <QFileInfo>

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
    Util::showFileInExplorer(m_file);
}

} // namespace Recording
