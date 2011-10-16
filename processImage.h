#ifndef PROCESSIMAGE_H
#define PROCESSIMAGE_H

#include <QtGui>
#include "videodevice.h"
#include <cv.h>
#include <highgui.h>
#include "facedetect.h"
#include "facerecognize.h"
#include "thread.h"

class ProcessImage : public QWidget
{
    Q_OBJECT
public:
    ProcessImage(QWidget *parent=0);
    FaceDetect faceDetect;
    FaceRec faceRec;
    bool recFlag;
    bool smallImageDrawFlag;
    bool firstRecFlag;
    QImage *QfaceImage;
    vector<double> personConfi;
    int iperson;
    double averConfi;
    void start();
    int rs;
    VideoDevice *vd;
    ~ProcessImage();

private:
    friend class Thread;
    Thread *paintThread;
    vector<int> count;
    QPainter *painter;
    QLabel *label;
    QImage *frame;
    IplImage *cvimg;
    //QPixmap *frame;
    QTimer *timer;
    uchar *pp;
    uchar * p;
    unsigned int len;
    QVector<QRect> listRect;
    int vectorMax(vector<int> a);
    int convert_yuv_to_rgb_pixel(int y, int u, int v);
    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    QImage* convert_iplimage_to_qimage( IplImage *img);
    void convert_qimage_to_iplimage( QImage* qImage,IplImage* cvimg);
    void queryFrame();
    CvHaarClassifierCascade*  cascade;
    CvMemStorage* storage;
    IplImage* detect(IplImage *img);
    int nearest;
    QString fileName;

private slots:
    void paintEvent(QPaintEvent *);
    void display_error(QString err);
    void changeCascadeDefault();
    void changeCascadeAlt();
    void changeCascadeAlt2();
    void changeCascadeProfileface();
    void changeCascadeUpperbody();
    void changeCascadeFullbody();
    void changeCascadeFromFile();
    void faceLearn();
    void paintRun();

};

#endif
