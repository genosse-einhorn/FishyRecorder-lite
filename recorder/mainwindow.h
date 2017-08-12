#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    void recordingStateChanged(bool isRecording);
    void showAboutDialog();
};

#endif // MAINWINDOW_H
