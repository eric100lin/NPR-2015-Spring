#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QProgressDialog>
#include <QAbstractButton>
#include "VideoProcessor.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace Ui {
class MainWindow;
}

class VideoProcessor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_btnPlay_clicked();
    void on_btnStop_clicked();
    void on_progressSlider_valueChanged(int value);
    void on_frameSpinBox_valueChanged(double newValue);
    void on_loopCheckBox_clicked(bool checked);
    void on_buttonGroupR_buttonClicked(QAbstractButton *button);
    void on_buttonGroupD_buttonClicked(QAbstractButton *button);
    void on_functionToolBox_currentChanged(int index);
    void on_cbShowCorrD_stateChanged(int state);
    void enablePbCopySplinesAcrossTime(bool enable);
    void drawModuleSelectChanged();
    void drawModuleCurrColorChanged();

    //Slots For VideoProcessor
    void showFrame(long index, cv::Mat frame);  // show a frame
    void sleep(int msecs);                      // sleep for a while
    void revert();                              // revert playing
    void updateBtn();                           // update button status
    void updateProgressBar();                   // update progress bar
    void updateProcessProgress(const std::string &message, int value);  // update process progress
    void closeProgressDialog();                 // canceling the process
    void pickColor();

private:
    Ui::MainWindow *ui;
    QProgressDialog *progressDialog;            // Process progress
    void setupToolBox();
    void setupFrameViewer();
    void updateTimeLabel();
    QString getFormattedTime(double timeInMS);
    void updateFrameSpinBox(long curPos);
    void enableVideoUI(bool vi);

    VideoProcessor *video;                      // video processor instance
    void setupVideoProcessor();
    bool loadFile(const QString &fileName);     // load file
};

#endif // MAINWINDOW_H
