#include <QtGui>
#include "facedetect.h"

FaceDetect::FaceDetect()
{
    cascadeFile = "haarcascade_frontalface_alt2.xml";
    cascade = (CvHaarClassifierCascade *) cvLoad(cascadeFile.toUtf8());
    flags = CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_DO_ROUGH_SEARCH;

    // Storage for the rectangles detected
    storage = cvCreateMemStorage(0);
    CvSize size;
    size.width = 70;
    size.height = 80;
    faceImage = cvCreateImage(size,IPL_DEPTH_8U,CV_8UC1);

}

FaceDetect::~FaceDetect() {
    if(cascade) cvReleaseHaarClassifierCascade(&cascade);
}


// Load a new classifier cascade, we unload first the previous classifier
void FaceDetect::setCascadeFile(QString cascadeFrom) {
    cascadeFile = cascadeFrom;
    if(cascade) cvReleaseHaarClassifierCascade(&cascade);
    cascade = (CvHaarClassifierCascade *) cvLoad(cascadeFile.toUtf8());
}

QString FaceDetect::cascadeName() const {
    return cascadeFile;
}

/* Possible values for flags on cvHaarDetectObjects. It can be a combination of zero or more of the following values:

        * CV_HAAR_SCALE_IMAGE- for each scale factor used the function will downscale the image rather than "zoom"
            the feature coordinates in the classifier cascade. Currently, the option can only be used alone,
            i.e. the flag can not be set together with the others.
        * CV_HAAR_DO_CANNY_PRUNING- If it is set, the function uses Canny edge detector to reject some image regions
            that contain too few or too much edges and thus can not contain the searched object. The particular
            threshold values are tuned for face detection and in this case the pruning speeds up the processing.
        * CV_HAAR_FIND_BIGGEST_OBJECT- If it is set, the function finds the largest object (if any) in the image.
            That is, the output sequence will contain one (or zero) element(s).
        * CV_HAAR_DO_ROUGH_SEARCH- It should be used only when CV_HAAR_FIND_BIGGEST_OBJECT is set and min_neighbors > 0.
            If the flag is set, the function does not look for candidates of a smaller size as soon as it has found the
            object (with enough neighbor candidates) at the current scale. Typically, when min_neighbors is fixed, the
            mode yields less accurate (a bit larger) object rectangle than the regular single-object mode
            (CV_HAAR_FIND_BIGGEST_OBJECT), but it is much faster, up to an order of magnitude. A greater value of
            min_neighbors may be specified to improve the accuracy.

Note, that in single-object mode CV_HAAR_DO_CANNY_PRUNING does not improve performance much and can even slow down the
processing. */



void FaceDetect::setFlags(int flagsFrom) {
    flags = flagsFrom;
}

QVector<QRect> FaceDetect::detectFaces(IplImage *cvImage) {
    QVector<QRect> listRect;
    CvRect *rect = NULL;
    double scale = 1.2;

    // Create a gray scale image (1 channel) to turn it into a small image that we send to cvHaarDetectObjects to process
    IplImage *grayImage = cvCreateImage(cvSize(cvImage->width, cvImage->height), cvImage->depth, CV_8UC1);
    IplImage *smallImage = cvCreateImage(cvSize(cvRound(cvImage->width/scale), cvRound(cvImage->height/scale)),
                                         cvImage->depth, CV_8UC1);

    cvCvtColor(cvImage, grayImage, CV_BGR2GRAY);
    cvResize(grayImage, smallImage);
    cvEqualizeHist(smallImage, smallImage);         // Grays smoothing (normaliza brillo, incrementa contraste)


    if(cascade) {                                  // It isn't necessary in this context, because cascade exist if we reach this point
        double timeElapsed = (double)cvGetTickCount();
        CvSeq *faces = cvHaarDetectObjects(smallImage, cascade, storage, scale, 0, flags, cvSize(30, 30));
        timeElapsed = (double)cvGetTickCount() - timeElapsed;
        printf( "detection time = %g ms\n", timeElapsed/((double)cvGetTickFrequency()*1000) );
        //qDebug() << QString("detection time = %1").arg(timeElapsed/((double)cvGetTickFrequency()*1000));
        for(int i = 0; i < faces->total; i++) {
            rect = (CvRect*)cvGetSeqElem(faces, i);
            listRect.append(QRect(rect->x * scale, rect->y * scale, rect->width * scale, rect->height * scale));
            region = *rect;
//            region.x = region.x*scale + 15;
//            region.y = region.y*scale + 10;
//            region.width = region.width*scale - 2*15;
//            region.height = region.height*scale - 10;
                        region.x = region.x*scale + 6;
                        region.y = region.y*scale + 5;
                        region.width = region.width*scale - 2*6;
                        region.height = region.height*scale - 5;
            if (region.width < 1)
                region.width = 1;
            if (region.height < 1)
                region.height = 1;
            printf("%dX%d\n",region.width,region.height);
            cvSetImageROI(grayImage,region);
            cvResize(grayImage,faceImage,CV_INTER_AREA);
            cvEqualizeHist(faceImage,faceImage);
            //saveByClick();
//            saveTimer = new QTimer(this);
//            connect(saveTimer,SIGNAL(timeout()),this,SLOT(saveFaceImage() ));
//            saveTimer->start(2000);

        }
    }

    cvReleaseImage(&grayImage);
    cvReleaseImage(&smallImage);

    return listRect;
}
