#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QThread;

namespace Ui {
class MainWindow;
}

namespace Recording {
class Coordinator;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    Recording::Coordinator *m_recorder;
    QThread                *m_recorderThread;

    void recordingStateChanged(bool isRecording);
    void showAboutDialog();
    void showAlDebugDialog();
};

#endif // MAINWINDOW_H
