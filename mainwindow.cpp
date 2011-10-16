#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <vector>
#define TOTAL 15
using namespace std;

class Thread;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    num1 = 0;
    num2 = 0;
    iImage =1;
    ui->setupUi(this);
    ui->abandonButton->setVisible(false);
    myCom = new Posix_QextSerialPort("/dev/ttyUSB0",QextSerialBase::Polling);
    myCom->open(QIODevice::ReadWrite);
       //以读写方式打开串口
    myCom->setBaudRate(BAUD9600);
       //波特率设置，我们设置为9600
    myCom->setDataBits(DATA_8);
       //数据位设置，我们设置为8位数据位
    myCom->setParity(PAR_NONE);
      //奇偶校验设置，我们设置为无校验
    myCom->setStopBits(STOP_1);
       //停止位设置，我们设置为1位停止位
    myCom->setFlowControl(FLOW_OFF);
       //数据流控制设置，我们设置为无数据流控制
    myCom->setTimeout(1);
       //延时设置，我们设置为延时500ms,这个在Windows下好像不起作用
    readTimer = new QTimer(this);
    readTimer->start(100);
           //设置延时为100ms
    connect(readTimer,SIGNAL(timeout()),this,SLOT(readMyCom()));
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendButtonClicked()));
         //信号和槽函数关联，延时一段时间，进行读串口操作
    connect(ui->actionDefault,SIGNAL(triggered()),ui->image,SLOT(changeCascadeDefault() ) );
    connect(ui->actionAlt,SIGNAL(triggered()),ui->image,SLOT(changeCascadeAlt() ) );
    connect(ui->actionAlt2,SIGNAL(triggered()),ui->image,SLOT(changeCascadeAlt2() ) );
    connect(ui->actionProfileface,SIGNAL(triggered()),ui->image,SLOT(changeCascadeProfileface() ) );
    connect(ui->actionUpperbody,SIGNAL(triggered()),ui->image,SLOT(changeCascadeUpperbody() ) );
    connect(ui->actionFullbody,SIGNAL(triggered()),ui->image,SLOT(changeCascadeFullbody() ) );
    connect(ui->actionFrom,SIGNAL(triggered()),ui->image,SLOT(changeCascadeFromFile() ) );
    connect(ui->captureButton,SIGNAL(clicked()),this,SLOT(toggleCaptureButton() ) );
    connect(ui->recButton,SIGNAL(clicked()),this,SLOT(toggleRecButton() ) );
    connect(ui->abandonButton,SIGNAL(clicked()),this,SLOT(abandonImage() ) );
    connect(ui->trainButton,SIGNAL(clicked()),ui->image,SLOT(faceLearn() ) );
    connect(ui->actionFullScreen,SIGNAL(triggered()),this,SLOT(showFullScreen() ) );
    connect(ui->actionNormalSize,SIGNAL(triggered()),this,SLOT(showNormal() ) );
    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),ui->spinBox,SLOT(setValue(int) ) );
    connect(ui->spinBox,SIGNAL(valueChanged(int)),ui->horizontalSlider,SLOT(setValue(int) ) );
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setThreshold(int) ) );
    connect(ui->actionRest,SIGNAL(triggered()),this,SLOT(reset() ) );
    connect(ui->actionUnload,SIGNAL(triggered()),this,SLOT(unloadTrainingdata() ) );
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(about() ) );
    countTimer = new QTimer(this);
    connect(countTimer,SIGNAL(timeout()),this,SLOT(login() ) );
    countTimer->start(4000);

    ui->image->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::toggleCaptureButton(){
    if (iImage != (TOTAL+1) && iImage != 1 )
    ui->trainButton->setDisabled(true);
    else ui->trainButton->setDisabled(false);
    if (ui->captureButton->text() == "Capture"){
        if (iImage == (TOTAL+1) ){
            QMessageBox::warning(this,tr("Alert"),tr("Training images are enough."),QMessageBox::Yes);
            return;
        }
        ui->captureButton->setText(tr("Add"));
        ui->abandonButton->setVisible(true);

        QDir dataDir;
        if (!dataDir.exists("data"))
            dataDir.mkdir("data");
        saveFaceImage();
//        if (num == 0)
//            sprintf(trainImageName,"data/1_%d.pgm",iImage); //To fix a little bug.
//        else

    }
    else {
        appendTrainFile();
        ui->captureButton->setText(tr("Capture"));
        ui->abandonButton->setVisible(false);
        ui->image->smallImageDrawFlag = false;
    }
}

void MainWindow::abandonImage(){
    ui->captureButton->setText(tr("Capture"));
    ui->abandonButton->setVisible(false);
    ui->image->smallImageDrawFlag = false;
    iImage -= 1;
}

void MainWindow::saveFaceImage(){
    QFileInfo infoFile("40.txt");
    if (!infoFile.exists()){
        system("touch 40.txt");
    }
    QFile file("40.txt");
    if (!file.open(QIODevice::ReadOnly)){
        printf("Cannot open file.\n");
        return;
    }
    QString line;
    char showNumber[256];
    QTextStream out(&file);
//    if (i == 0)
        while (!out.atEnd()){
        line = out.readLine();
        }
    if (num1 ==0)
    num1 = line.split(" ").at(0).toInt() + 1;
    sprintf(trainImageName,"data/%d_%d.pgm",num1,iImage);
    cvSaveImage(trainImageName,ui->image->faceDetect.faceImage);
    iImage++;
    sprintf(showNumber,"%s saved,total %d images saved",trainImageName,iImage-1);
    ui->statusbar->showMessage(showNumber,5000);
    if (ui->image->QfaceImage->load(trainImageName) ){
        *ui->image->QfaceImage = ui->image->QfaceImage->scaledToHeight(60);
         ui->image->smallImageDrawFlag = true;
    }
}

void MainWindow::appendTrainFile(){
    QFile file("40.txt");
    if (!file.open(QIODevice::ReadWrite)){
        printf("Cannot open file.\n");
        return;
    }
    QTextStream out(&file);
    if (num2 == 0){
        QString line;
        while (!out.atEnd()){
        line = out.readLine();
        }
        num2 = line.split(" ").at(0).toInt() + 1;
        trainNum = QString::number(num2);
   //     ui->statusbar->showMessage(trainNum);
    }else{
        out.readAll();    //Make the iterator point to the end
    }
    ui->statusbar->showMessage(trainNum);
    if (ui->comboBox->currentText().isEmpty()){
        QMessageBox::warning(this,tr("Alert"),
                                  tr("Please input your name."),
                                  QMessageBox::Yes);
        iImage--;
        return;
    }else{
        out << trainNum
            << " "
            << ui->comboBox->currentText()
            << " "
            << trainImageName
            << endl;
    }
}

void MainWindow::toggleRecButton(){
    if (ui->recButton->text() == "Rec" ){
        ui->recButton->setText(tr("Stop"));
        ui->image->recFlag = true;
    }
    else {
        ui->recButton->setText(tr("Rec"));
        ui->image->recFlag = false;
    }
}

void MainWindow::about(){
    QMessageBox::about(this,tr("About QTFaceRec"),
                          tr("<h2>QTFaceRec 1.0</h2>"
                             "<h4>By 11</h4>"
                             "<p>Based on OpenCV 2.3 and Shervin's program:<br/>"
                             "www.shervinemami.co.cc/facerecognition.html<br/>"
                             "</p>"));
}

//////////////////////////////////////////////////////
/////////串口
void MainWindow::readMyCom()
{
    QByteArray temp = myCom->readAll();
    //读取串口缓冲区的所有数据给临时变量temp
    ui->textBrowser->insertPlainText(temp);
//   cout << temp << "END" << endl;
    //将串口的数据显示在窗口的文本浏览器中
}

void MainWindow::sendButtonClicked()
{
   myCom->write(ui->comboBox->currentText().toAscii());
   //以ASCII码形式将数据写入串口
}
/////////串口
////////////////////////////////////////////

void MainWindow::login()
{
//    printf("%lf\n",(double) ui->image->faceRec.recCount / (double) ui->image->faceRec.iCount);
//    if (ui->image->faceRec.iCount > 3
//       && (double) ui->image->faceRec.recCount / (double) ui->image->faceRec.iCount > 0.6){
    float temp;
    if (ui->image->personConfi.size() > 5 && !ui->image->personConfi.empty()){
        ui->image->averConfi = ui->image->faceRec.average(ui->image->personConfi);
        ui->image->personConfi.clear();
        ui->image->firstRecFlag = true;
    }
    if (ui->image->recFlag && ui->image->averConfi > ui->image->faceRec.threshold)
    {
        QString serial="1";
        myCom->write(serial.toAscii());
        ui->image->faceRec.iCount = 0;
        temp = ui->image->averConfi;
        ui->image->averConfi = 0;
        cvZero(ui->image->faceDetect.faceImage);
        QMessageBox::warning(this,tr("Logged in"),tr("You have logged in!\nConfidence=%1").arg(temp),QMessageBox::Yes);
    }
//    ui->image->faceRec.recCount = 0;
    if (ui->image->faceRec.iCount > 200){
        QMessageBox::warning(this,tr("Failed"),tr("Login failed!\nYour face has been recorded!"),QMessageBox::Yes);
        ui->image->faceRec.iCount   = 0;
    }
}

void MainWindow::setThreshold(int thresholdFrom)
{
    ui->image->faceRec.threshold = (float) thresholdFrom/100.0f ;
}

void MainWindow::reset()
{
    system("cp 40.txt.bak 40.txt");
    iImage = 1;
}

void MainWindow::unloadTrainingdata()
{
     ui->image->faceRec.recCount           = 0;
     ui->image->faceRec.iCount             = 0;
     ui->image->faceRec.personNumTruthMat = 0;      // array of person numbers
     ui->image->faceRec.nTrainFaces               = 0;    // the number of training images
     ui->image->faceRec.nEigens                   = 0;      // the number of eigenvalues
     ui->image->faceRec.pAvgTrainImg       = 0;      // the average image
     ui->image->faceRec.eigenVectArr      = 0;      // eigenvectors
     ui->image->faceRec.eigenValMat           = 0;      // eigenvalues
     ui->image->faceRec.projectedTrainFaceMat = 0;      // projected training faces
     ui->image->faceRec.trainPersonNumMat = 0;  // the person numbers during training
     ui->image->faceRec.projectedTestFace = 0;   // projected test faces
     ui->image->faceRec.threshold         = 0.85f;
}
