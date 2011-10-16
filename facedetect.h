#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QtGui>
#include <QString>
#include <QVector>
#include <QRect>
#include <QTimer>
#include <cv.h>
#include <highgui.h>

class FaceDetect {
public:
    FaceDetect();
    ~FaceDetect();

    void setCascadeFile(QString cascadeFile);
    QString cascadeName() const;
    void setFlags(int flags);
    QVector<QRect> detectFaces(IplImage *cvImage);
    CvRect region;
    IplImage *faceImage;    

private:
    CvHaarClassifierCascade *cascade;
    CvMemStorage *storage;
    int flags;
    QString cascadeFile;
    QTimer saveTimer;
    CvSize size;

//    void saveByClick();
};

#endif // FACEDETECT_H
