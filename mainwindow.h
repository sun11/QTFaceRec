#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "posix_qextserialport.h"
#include <QMainWindow>
#include <QTimer>
#include <QTextStream>
#include <vector>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    int iImage;
    char trainImageName[256];
    Ui::MainWindow *ui;
    Posix_QextSerialPort *myCom;
    QTimer *readTimer;
    QTimer *countTimer;
    QString trainNum;
    QTextStream out;
    int num1;    // the person number in training file
    int num2;

private slots:
    void toggleCaptureButton();
    void toggleRecButton();
    void appendTrainFile();
    void abandonImage();
    void sendButtonClicked();
    void setThreshold(int thresholdFrom);
    void readMyCom();
    void saveFaceImage();
    void login();
    void reset();
    void about();
    void unloadTrainingdata();
};

#endif // MAINWINDOW_H
