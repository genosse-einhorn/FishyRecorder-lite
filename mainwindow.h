#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class RecordingCoordinator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void recordToggle(bool record);

private:
    Ui::MainWindow *ui;

    RecordingCoordinator *m_recorder;
};

#endif // MAINWINDOW_H
