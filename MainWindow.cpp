#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QHBoxLayout>
#include <QIcon>
#include <QColorDialog>
#include "RotoscopeModule.h"
#include "DrawModule.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupVideoProcessor();
    setupToolBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupVideoProcessor()
{
    video = new VideoProcessor;
    connect(video, SIGNAL(showFrame(long,cv::Mat)), this, SLOT(showFrame(long,cv::Mat)));
    connect(video, SIGNAL(sleep(int)), this, SLOT(sleep(int)));
    connect(video, SIGNAL(revert()), this, SLOT(revert()));
    connect(video, SIGNAL(updateBtn()), this, SLOT(updateBtn()));
    connect(video, SIGNAL(updateProgressBar()), this, SLOT(updateProgressBar()));
    connect(video, SIGNAL(updateProcessProgress(std::string, int)), this, SLOT(updateProcessProgress(std::string, int)));
    connect(video, SIGNAL(closeProgressDialog()), this, SLOT(closeProgressDialog()));
}

void MainWindow::setupToolBox()
{
    QPixmap newLook(22, 22);
    QRgb initC = qRgb(0,0,0);
    newLook.fill(initC);
    ui->tbColor->setIcon(QIcon(newLook));
    ui->tbColor->setText("Current color");
    ui->tbColor->setToolTip("Current color");
    connect(ui->tbColor, SIGNAL(clicked()), this, SLOT(pickColor()));
}

void MainWindow::pickColor()
{
    QRgb initC = ui->frameWidget->draw->getCurrColor();
    QColor col = QColorDialog::getColor(initC);
    if (!col.isValid())
        return;
    Vec3f myCol(float(col.red()) / 255.f, float(col.green()) / 255.f,
            float(col.blue()) / 255.f);
    ui->frameWidget->draw->setColor(myCol);

    drawModuleCurrColorChanged();
}

//Slots For MainWindow
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Video"),
                                                    ".",
                                                    tr("Video Files (*.avi *.mov *.mpeg *.mp4)"));
    if(!fileName.isEmpty())
    {
        if(loadFile(fileName))
        {
            enableVideoUI(true);
            return;
        }
    }
    enableVideoUI(false);
}

void MainWindow::on_btnPlay_clicked()
{
    bool isStop = video->isStop();
    if(isStop && ui->btnPlay->isChecked())
    {
        ui->progressSlider->blockSignals(true);
        ui->frameSpinBox->blockSignals(true);
        ui->progressSlider->setEnabled(false);
        ui->frameSpinBox->setEnabled(false);
        video->setDelay(1000 / video->getFrameRate());
        video->playIt();
    }
    else if(!isStop && !ui->btnPlay->isChecked())
    {
        ui->progressSlider->blockSignals(false);
        ui->frameSpinBox->blockSignals(false);
        ui->progressSlider->setEnabled(true);
        ui->frameSpinBox->setEnabled(true);
        video->pauseIt();
    }
}

void MainWindow::on_btnStop_clicked()
{
    video->stopIt();
}

void MainWindow::on_progressSlider_valueChanged(int position)
{
    long pos = position * video->getLength() /
            ui->progressSlider->maximum();
    video->jumpTo(pos);

    ui->frameSpinBox->blockSignals(true);
    updateFrameSpinBox(pos);
    ui->frameSpinBox->blockSignals(false);

    updateTimeLabel();
}

void MainWindow::on_frameSpinBox_valueChanged(double newValue)
{
    long curPos = (long)newValue;
    video->jumpTo(curPos);

    ui->progressSlider->blockSignals(true);
    ui->progressSlider->setValue(curPos
                                 * ui->progressSlider->maximum()
                                 / video->getLength() * 1.0);
    ui->progressSlider->blockSignals(false);

    updateTimeLabel();
}

void MainWindow::on_loopCheckBox_clicked(bool checked)
{
    video->setLoop(checked);
}

void MainWindow::on_buttonGroupR_buttonClicked(QAbstractButton *button)
{
    RotoscopeModule *roto = ui->frameWidget->roto;
    if (button == ui->pbMenuEditR)
        roto->toolChange(T_MANUAL);
    else if (button == ui->pbSelectR)
        roto->toolChange(T_SELECT);
    else if (button == ui->pbNudgeR)
        roto->toolChange(T_NUDGE);
    else if (button == ui->pbCopySplinesAcrossTime)
        roto->copySplinesAcrossTime();
    else if (button == ui->pbImproveTrack)
    {
        bool startOver = false;
        roto->rejigWrapper(startOver);
    }
}

void MainWindow::on_buttonGroupD_buttonClicked(QAbstractButton *button)
{
    DrawModule *draw = ui->frameWidget->draw;
    if (button == ui->pbDrawD)
        draw->toolChange(D_DRAW);
    else if (button == ui->pbDropperD)
        draw->toolChange(D_DROPPER);
    else if (button == ui->pbSelectD)
        draw->toolChange(D_SELECT);
    else if (button == ui->pbPropagateD)
        draw->propagatedStroke();
}

void MainWindow::on_functionToolBox_currentChanged(int index)
{
    RotoscopeModule *roto = ui->frameWidget->roto;
    DrawModule *draw = ui->frameWidget->draw;
    if(index==0)
        ui->frameWidget->changeModules(roto);
    else if(index==3)
        ui->frameWidget->changeModules(draw);
    else
        ui->frameWidget->changeModules(NULL);
    ui->frameWidget->updateGL();
}

void MainWindow::on_cbShowCorrD_stateChanged(int state)
{
    ui->frameWidget->draw->toggleShow(state);
}

void MainWindow::enablePbCopySplinesAcrossTime(bool enable)
{
    ui->pbCopySplinesAcrossTime->setEnabled(enable);
    ui->pbImproveTrack->setEnabled(enable);
}

void MainWindow::drawModuleSelectChanged()
{

}

void MainWindow::drawModuleCurrColorChanged()
{
    QRgb rgb = ui->frameWidget->draw->getCurrColor();
    QPixmap newLook(22, 22);
    newLook.fill(rgb);
    ui->tbColor->setIcon(QIcon(newLook));
}

//Slots For VideoProcessor
void MainWindow::showFrame(long index, cv::Mat frame)
{
    cv::Mat temp; // make the same cv::Mat
    cvtColor(frame, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage img = QImage((const unsigned char*)(temp.data),
                        temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    ui->frameWidget->showFrame(index,img);
}

void MainWindow::sleep(int msecs)
{
    QTime dieTime = QTime::currentTime().addMSecs(msecs);
    while(QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::updateBtn()
{
    bool isStop = video->isStop();
    ui->btnStop->setEnabled(!isStop);
    if(isStop && ui->btnPlay->isChecked())
        ui->btnPlay->setChecked(false);
    else if(!isStop && !ui->btnPlay->isChecked())
        ui->btnPlay->setChecked(true);
}

void MainWindow::updateProgressBar()
{
    long curPos = video->getNumberOfPlayedFrames();

    // update the spin box
    updateFrameSpinBox(curPos);

    // update the progress bar
    ui->progressSlider->setValue(curPos
                                 * ui->progressSlider->maximum()
                                 / video->getLength() * 1.0);

    // update the time label
    updateTimeLabel();
}

void MainWindow::updateProcessProgress(const std::string &message, int value)
{
    if(!progressDialog){
        progressDialog = new QProgressDialog(this);
        progressDialog->setLabelText(QString::fromStdString(message));
        progressDialog->setRange(0, 100);
        progressDialog->setModal(true);
        progressDialog->setCancelButtonText(tr("Abort"));
        progressDialog->show();
        progressDialog->raise();
        progressDialog->activateWindow();
    }
    progressDialog->setValue(value + 1);
    qApp->processEvents();
    if (progressDialog->wasCanceled()){
        video->stopIt();
    }
}

void MainWindow::closeProgressDialog()
{
    progressDialog->close();
    progressDialog = 0;
}

bool MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("VideoPlayer"),
                             tr("Unable to load file %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    // change the cursor
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // input file
    if (!video->setInput(fileName.toStdString())){
        QMessageBox::warning(this, tr("VideoPlayer"),
                             tr("Unable to load file %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    // set the size of GL frameviewer
    setupFrameViewer();

    // update the time label
    updateTimeLabel();

    // set the current file location
    QString curFile = QFileInfo(fileName).canonicalPath();
    setWindowTitle(curFile);

    // restore the cursor
    QApplication::restoreOverrideCursor();
    return true;
}

void MainWindow::setupFrameViewer()
{
    //Resize window for video frmae
    cv::Size frameSize = video->getFrameSize();
    QSize frameWidgetSize(frameSize.width,frameSize.height);
    ui->frameWidget->setMinimumSize(frameWidgetSize);
    ui->frameWidget->setFocusPolicy(QWidget::focusPolicy());

    QSize videoAreaSize(frameSize.width,frameSize.height+50);
    ui->videoArea->setMinimumSize(videoAreaSize);

    //Show the first video frmae
    cv::Mat firstFrame;
    if(video->getNextFrame(firstFrame))
        showFrame(0, firstFrame);

    //Set up maximum frame number
    long videoLength = video->getLength();
    ui->frameSpinBox->setMaximum(videoLength-1);
    ui->labelOrgFrameNumber->setText(QString::number(videoLength));
    ui->frameWidget->setUpModules(video->getClonedCapture(),(int)videoLength);
    connect(ui->frameWidget->roto, SIGNAL(enablePbCopySplinesAcrossTime(bool)),
            this, SLOT(enablePbCopySplinesAcrossTime(bool)));
    connect(ui->frameWidget->draw, SIGNAL(selectChanged()),
            this, SLOT(drawModuleSelectChanged()));
    connect(ui->frameWidget->draw, SIGNAL(currColorChanged()),
            this, SLOT(drawModuleCurrColorChanged()));
}

void MainWindow::updateTimeLabel()
{
    QString curPos = getFormattedTime(video->getPositionMS());
    ui->currentTimeLabel->setText(curPos);

    QString length = getFormattedTime(video->getLengthMS());
    ui->totalTimeLabel->setText(length);
}

QString MainWindow::getFormattedTime(double timeInMS)
{
    int timeInSeconds = (int)(timeInMS/1000.0);
    int seconds = (int) (timeInSeconds) % 60 ;
    int minutes = (int) ((timeInSeconds / 60) % 60);
    int hours   = (int) ((timeInSeconds / (60*60)) % 24);

    QTime t(hours, minutes, seconds);
    if (hours == 0 )
        return t.toString("mm:ss");
    else
        return t.toString("h:mm:ss");
}

void MainWindow::updateFrameSpinBox(long curPos)
{
    ui->frameSpinBox->setValue(curPos);
}

void MainWindow::revert()
{
    updateBtn();
}

void MainWindow::enableVideoUI(bool vi)
{
    ui->frameWidget->setEnabled(vi);
    ui->menuPlay->setEnabled(vi);
    ui->loopCheckBox->setEnabled(vi);
    ui->progressSlider->setEnabled(vi);
    ui->btnPlay->setEnabled(vi);
    ui->btnStop->setEnabled(vi);
    ui->frameSpinBox->setEnabled(vi);
    ui->functionToolBox->setEnabled(vi);
    ui->pageRotoscoping->setEnabled(vi);
    if(!vi){
        ui->progressSlider->setValue(0);
    }
}
