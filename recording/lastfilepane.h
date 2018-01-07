#ifndef RECORDING_LASTFILEPANE_H
#define RECORDING_LASTFILEPANE_H

#include <QWidget>

namespace Recording {

namespace Ui {
class LastFilePane;
}

class LastFilePane : public QWidget
{
    Q_OBJECT

public:
    explicit LastFilePane(QWidget *parent = 0);
    ~LastFilePane();

public slots:
    void newRecordingFile(const QString &file);

private:
    Ui::LastFilePane *ui;

    QString m_file;

    void showFileInExplorer();
};


} // namespace Recording
#endif // RECORDING_LASTFILEPANE_H
