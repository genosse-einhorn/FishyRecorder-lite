#include "thirdpartylicensedialog.h"
#include "ui_thirdpartylicensedialog.h"

#include <mxe-licenses-html.h>
#include <zlib.h>

ThirdPartyLicenseDialog::ThirdPartyLicenseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ThirdPartyLicenseDialog)
{
    ui->setupUi(this);

    char license_str[MXE_LICENSE_HTML_LENGTH] = {};
    uLongf license_str_size = sizeof(license_str);
    ::uncompress((unsigned char*)license_str, &license_str_size, mxe_license_html_compressed, sizeof(mxe_license_html_compressed));

    ui->textBrowser->setHtml(QString::fromUtf8(license_str, sizeof(license_str)));
}

ThirdPartyLicenseDialog::~ThirdPartyLicenseDialog()
{
    delete ui;
}
