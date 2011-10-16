#ifndef FACERECOGNIZE_H
#define FACERECOGNIZE_H

// eigenface.c, by Robin Hewitt, 2007
//
// Example program showing how to implement eigenface with OpenCV

// Usage:
//
// First, you need some face images. I used the ORL face database.
// You can download it for free at
//    www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html
//
// List the training and test face images you want to use in the
// input files train.txt and test.txt. (Example input files are provided
// in the download.) To use these input files exactly as provided, unzip
// the ORL face database, and place train.txt, test.txt, and eigenface.exe
// at the root of the unzipped database.
//
// To run the learning phase of eigenface, enter
//    eigenface train
// at the command prompt. To run the recognition phase, enter
//    eigenface test
#include <QFileInfo>
#include <stdio.h>
#include <string>
//#include <string>
#include <vector>
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"
using namespace std;

class FaceRec {
public:
    FaceRec();
    ~FaceRec();

    int recCount;                     // count the times of recognizing people successfully
    int iCount;                       // the total times of count
    IplImage ** faceImgArr;         // array of face images
    CvMat    *  personNumTruthMat;  // array of person numbers
    int nTrainFaces;               // the number of training images
    int nEigens;                   // the number of eigenvalues
    int nPersons;                  // the number of people in the training set
    vector<string> personNames;
    IplImage * pAvgTrainImg;       // the average image
    IplImage ** eigenVectArr;      // eigenvectors
    CvMat * eigenValMat;           // eigenvalues
    CvMat * projectedTrainFaceMat; // projected training faces
    CvMat * trainPersonNumMat;  // the person numbers during training
    float * projectedTestFace;
    float fConfidence;
    float threshold;

    void learn();
    void recognize();
    int recFromFrame(IplImage *faceImage);
    double average(vector<double> personConfi);
    void unloadTrainingdata();

private:
    void doPCA();
    void storeTrainingData();
    int  loadTrainingData(CvMat ** pTrainPersonNumMat);
    int  findNearestNeighbor(float * projectedTestFace);
    int  loadFaceImgArray(const char * filename);

};
#endif // FACERECOGNIZE_H
