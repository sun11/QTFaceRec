#include <QtGui>
#include "processImage.h"
#include "videodevice.h"
#include <string>
#include <vector>
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
}
using namespace std;

ProcessImage::ProcessImage(QWidget *parent):QWidget(parent)
{
    vector<int> count(256,0);
    personConfi.clear();
    firstRecFlag = true;
    averConfi = -100.0f;
    pp = (unsigned char *)malloc(WIDTH * HEIGHT/*QWidget::width()*QWidget::height()*/* 3 * sizeof(char));
  //painter = new QPainter(this);
    frame = new QImage(pp,WIDTH,HEIGHT,QImage::Format_RGB888);
    QfaceImage = new QImage;
    int width = frame->width();
    int height = frame->height();
    CvSize Size;
    Size.height = height;
    Size.width = width;
    recFlag = false;
    smallImageDrawFlag =  false;
    cvimg = cvCreateImage(Size, IPL_DEPTH_8U, 3);
   // frame = new QPixmap(640,WIDTH);
    label = new QLabel();
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(label);
    setLayout(hLayout);
   // connect(paintThread,SIGNAL(sigtest() ),this,SLOT(update() ) );
    vd = new VideoDevice(tr("/dev/video2"));
//    connect(vd, SIGNAL(display_error(QString)), this,SLOT(display_error(QString)));
    rs = vd->open_device();
    if(-1==rs)
    {
        QMessageBox::warning(this,tr("error"),tr("open /dev/dsp error"),QMessageBox::Yes);
        vd->close_device();
    }

    rs = vd->init_device();
    if(-1==rs)
    {
        QMessageBox::warning(this,tr("error"),tr("init failed"),QMessageBox::Yes);
        vd->close_device();
    }

    rs = vd->start_capturing();
    if(-1==rs)
    {
        QMessageBox::warning(this,tr("error"),tr("start capture failed"),QMessageBox::Yes);
        vd->close_device();
    }

    if(-1==rs)
    {
        QMessageBox::warning(this,tr("error"),tr("get frame failed"),QMessageBox::Yes);
        vd->stop_capturing();
    }
    paintThread = new Thread;
    connect(paintThread,SIGNAL(sigtest() ),this,SLOT(paintRun()),Qt::DirectConnection);
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(500);


}

ProcessImage::~ProcessImage()
{
    rs = vd->stop_capturing();
    rs = vd->uninit_device();
    rs = vd->close_device();
}

void ProcessImage::queryFrame()
{
    rs = vd->get_frame((void **)&p,&len);
    convert_yuv_to_rgb_buffer(p,pp,WIDTH,HEIGHT/*QWidget::width(),QWidget::height()*/);
    frame->loadFromData((uchar *)pp,/*len*/WIDTH * HEIGHT * 3 * sizeof(char));
    convert_qimage_to_iplimage(frame,cvimg);
    listRect = faceDetect.detectFaces(cvimg);
    QFileInfo facedata("facedata.xml");
    if (facedata.exists()
        && faceDetect.region.width >= 80
        && faceDetect.region.width <= 180
        && recFlag)
    nearest = faceRec.recFromFrame(faceDetect.faceImage);
}

void ProcessImage::paintRun()
{
    double timeCount = (double)cvGetTickCount();
    rs = vd->get_frame((void **)&p,&len);
    convert_yuv_to_rgb_buffer(p,pp,WIDTH,HEIGHT/*QWidget::width(),QWidget::height()*/);
    frame->loadFromData((uchar *)pp,/*len*/WIDTH * HEIGHT * 3 * sizeof(char));
    convert_qimage_to_iplimage(frame,cvimg);
    listRect = faceDetect.detectFaces(cvimg);
    QFileInfo facedata("facedata.xml");
    if (facedata.exists()
        && faceDetect.region.width >= 80
        && faceDetect.region.width <= 200
        && recFlag)
    {
        nearest = faceRec.recFromFrame(faceDetect.faceImage);
        if ((!firstRecFlag) && iperson != nearest)
            personConfi.push_back(0.5);
        else{
        personConfi.push_back(faceRec.fConfidence);
        if (firstRecFlag)
            iperson = nearest;
        firstRecFlag = false;
        }
    }
    rs = vd->unget_frame();
    timeCount = (double)cvGetTickCount() - timeCount;
    printf( "Frame show time = %g ms\n", timeCount/((double)cvGetTickFrequency()*1000) );
 }

void ProcessImage::start()
{
    paintThread->start();
}


void ProcessImage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen(Qt::green, 1, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
    painter.setPen(pen);
    //painter.begin(this);
    painter.drawImage(0,0,*frame);
    foreach(QRect rect, listRect) painter.drawRect(rect);

    if (smallImageDrawFlag ){
        painter.drawImage(0,0,*QfaceImage);
    }

    if (faceDetect.region.width >= 80
        && faceDetect.region.width <= 90
        && faceDetect.region.height >= 90
        && faceDetect.region.height <= 120)
        painter.drawText(faceDetect.region.x, faceDetect.region.y + 15,"Good Size");

    if (recFlag
       && faceDetect.region.width >= 80
       && faceDetect.region.width <= 200
       && faceRec.fConfidence > faceRec.threshold ){
        char showName[256];
        snprintf(showName,sizeof(showName)-1,"ID:%s",faceRec.personNames[nearest-1].c_str());
        QPen pen1(Qt::cyan,1, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin);
        painter.setPen(pen1);
        painter.drawText(faceDetect.region.x, faceDetect.region.y+faceDetect.region.height+15,showName);
    }
    painter.end();


    rs = vd->unget_frame();
    listRect.clear();

  //  label->setPixmap(QPixmap::fromImage(*frame,Qt::AutoColor));
//    image->save("cap.jpg");
   // label->show();
   // rs = vd->unget_frame();
   // label->drawFrame();

//        QPixmap *pixImage = new QPixmap();
//    pixImage->loadFromData((uchar *)pp,sizeof(pp),0,Qt::AutoColor);
//    painter=new   QPainter(this);
//    painter->begin(this);
//    painter->drawPixmap(0,0,QWidget::width(),QWidget::height(),*pixImage);
//    painter->end();
}

void ProcessImage::display_error(QString err)
{
    QMessageBox::warning(this,tr("error"), err,QMessageBox::Yes);
}


int ProcessImage::convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
 unsigned int in, out = 0;
 unsigned int pixel_16;
 unsigned char pixel_24[3];
 unsigned int pixel32;
 int y0, u, y1, v;
 for(in = 0; in < width * height * 2; in += 4) {
  pixel_16 =
   yuv[in + 3] << 24 |
   yuv[in + 2] << 16 |
   yuv[in + 1] <<  8 |
   yuv[in + 0];
  y0 = (pixel_16 & 0x000000ff);
  u  = (pixel_16 & 0x0000ff00) >>  8;
  y1 = (pixel_16 & 0x00ff0000) >> 16;
  v  = (pixel_16 & 0xff000000) >> 24;
  pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
  pixel_24[0] = (pixel32 & 0x000000ff);
  pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
  pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
  rgb[out++] = pixel_24[0];
  rgb[out++] = pixel_24[1];
  rgb[out++] = pixel_24[2];
  pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
  pixel_24[0] = (pixel32 & 0x000000ff);
  pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
  pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
  rgb[out++] = pixel_24[0];
  rgb[out++] = pixel_24[1];
  rgb[out++] = pixel_24[2];
 }
 return 0;
}

int ProcessImage::convert_yuv_to_rgb_pixel(int y, int u, int v)
{
 unsigned int pixel32 = 0;
 unsigned char *pixel = (unsigned char *)&pixel32;
 int r, g, b;
 r = y + (1.370705 * (v-128));
 g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
 b = y + (1.732446 * (u-128));
 if(r > 255) r = 255;
 if(g > 255) g = 255;
 if(b > 255) b = 255;
 if(r < 0) r = 0;
 if(g < 0) g = 0;
 if(b < 0) b = 0;
 pixel[0] = r * 220 / 256;
 pixel[1] = g * 220 / 256;
 pixel[2] = b * 220 / 256;
 return pixel32;
}

void ProcessImage::convert_qimage_to_iplimage(QImage* qImage,IplImage* cvimg)
{
    int width = qImage->width();
    int height = qImage->height();
    CvSize Size;
    Size.height = height;
    Size.width = width;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QRgb rgb = qImage->pixel(x, y);
            cvSet2D(cvimg, y, x, CV_RGB(qRed(rgb), qGreen(rgb), qBlue(rgb)));
        }
    }
}

QImage* ProcessImage::convert_iplimage_to_qimage(IplImage *cvImg)
{
    if(!cvImg)
    return NULL;
    //IplImage* temp=NULL;
    //temp=cvCloneImage(cvImg);
    cvCvtColor(cvImg,cvImg,CV_BGR2RGB);
    uchar* imgData = (uchar*)(cvImg->imageData);
    QImage *qImg=new QImage(imgData,cvImg->width,cvImg->height,QImage::Format_RGB888);
    return qImg;
    cvReleaseImage(&cvImg);
    delete imgData;
}

void ProcessImage::changeCascadeDefault()
{
    faceDetect.setCascadeFile("haarcascade_frontalface_default.xml");
}

void ProcessImage::changeCascadeAlt()
{
    faceDetect.setCascadeFile("haarcascade_frontalface_alt.xml");
}

void ProcessImage::changeCascadeAlt2()
{
    faceDetect.setCascadeFile("haarcascade_frontalface_alt2.xml");
}

void ProcessImage::changeCascadeProfileface()
{
    faceDetect.setCascadeFile("haarcascade_frontalface_profileface.xml");
}

void ProcessImage::changeCascadeUpperbody()
{
    faceDetect.setCascadeFile("haarcascade_upperbody.xml");
}

void ProcessImage::changeCascadeFullbody()
{
    faceDetect.setCascadeFile("haarcascade_fullbody.xml");
}

void ProcessImage::changeCascadeFromFile()
{
    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open cascade file"),"",
                                            tr("Cascade Files(*.xml)") );
    if (!fileName.isEmpty() )
            faceDetect.setCascadeFile(fileName);
}

void ProcessImage::faceLearn(){
    bool restoreFlag = false;
    system("./faceRec train 40.txt");
    if (recFlag){
        recFlag = false;
        restoreFlag = true;
    }
    faceRec.unloadTrainingdata();
    if (restoreFlag)
        recFlag = true;
//    faceRec.learn();
}

