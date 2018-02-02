#ifndef THIRDPARTYLICENSEDIALOG_H
#define THIRDPARTYLICENSEDIALOG_H

#include <QDialog>

namespace Ui {
class ThirdPartyLicenseDialog;
}

class ThirdPartyLicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThirdPartyLicenseDialog(QWidget *parent = 0);
    ~ThirdPartyLicenseDialog();

private:
    Ui::ThirdPartyLicenseDialog *ui;
};

#endif // THIRDPARTYLICENSEDIALOG_H
