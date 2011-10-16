#include "facerecognize.h"

FaceRec::FaceRec()
{
   // faceImgArr        = 0;      // array of face images
    recCount           = 0;
    iCount             = 0;
    personNumTruthMat = 0;      // array of person numbers
    nTrainFaces               = 0;      // the number of training images
    nEigens                   = 0;      // the number of eigenvalues
    pAvgTrainImg       = 0;      // the average image
    eigenVectArr      = 0;      // eigenvectors
    eigenValMat           = 0;      // eigenvalues
    projectedTrainFaceMat = 0;      // projected training faces
    trainPersonNumMat = 0;  // the person numbers during training
    projectedTestFace = 0;   // projected test faces
    threshold         = 0.85f;
    // load the saved training data and project the test images onto the PCA subspace
    QFileInfo facedata("facedata.xml");
    if (facedata.exists() ){
        if( !loadTrainingData( &trainPersonNumMat ) ) return;
        projectedTestFace = (float *)cvAlloc( nEigens*sizeof(float) );
    }
}

FaceRec::~FaceRec()
{
}

double FaceRec::average(vector<double> personConfi)
{
    double sum = 0;
    foreach(double confi,personConfi)   sum += confi;
    return sum/personConfi.size();
}

//////////////////////////////////
// learn()
//
void FaceRec::learn()
{
        int i, offset;

        // load training data
        nTrainFaces = loadFaceImgArray("train.txt");
        if( nTrainFaces < 2 )
        {
                fprintf(stderr,
                        "Need 2 or more training faces\n"
                        "Input file contains only %d\n", nTrainFaces);
                return;
        }

        // do PCA on the training faces
        doPCA();

        // project the training images onto the PCA subspace
        projectedTrainFaceMat = cvCreateMat( nTrainFaces, nEigens, CV_32FC1 );
        offset = projectedTrainFaceMat->step / sizeof(float);
        for(i=0; i<nTrainFaces; i++)
        {
                //int offset = i * nEigens;
                cvEigenDecomposite(
                        faceImgArr[i],
                        nEigens,
                        eigenVectArr,
                        0, 0,
                        pAvgTrainImg,
                        //projectedTrainFaceMat->data.fl + i*nEigens);
                        projectedTrainFaceMat->data.fl + i*offset);
        }

        // store the recognition data as an xml file
        storeTrainingData();
}


//////////////////////////////////
// recFromFram()
//recognize from a camera frame
int FaceRec::recFromFrame(IplImage *faceImage)
{
     QFileInfo facedata("facedata.xml");
     if (facedata.exists() && trainPersonNumMat == 0 && projectedTestFace == 0 ){
        loadTrainingData( &trainPersonNumMat ) ;
        projectedTestFace = (float *)cvAlloc( nEigens*sizeof(float) );
     }

        cvEigenDecomposite(
                        faceImage,
                        nEigens,
                        eigenVectArr,
                        0, 0,
                        pAvgTrainImg,
                        projectedTestFace);

        int iNearest,nearest;
        iNearest = findNearestNeighbor(projectedTestFace);
        nearest  = trainPersonNumMat->data.i[iNearest];
        printf("Nearest = %d\n", nearest);
        printf("Name = %s\n",personNames[nearest-1].c_str());
        return nearest;
}


//////////////////////////////////
// loadTrainingData()
//
int FaceRec::loadTrainingData(CvMat ** pTrainPersonNumMat)
{
        CvFileStorage * fileStorage;
        int i;

        // create a file-storage interface
        fileStorage = cvOpenFileStorage( "facedata.xml", 0, CV_STORAGE_READ );
        if( !fileStorage )
        {
                fprintf(stderr, "Can't open facedata.xml\n");
                return 0;
        }

        personNames.clear();
        nPersons = cvReadIntByName(fileStorage,0,"nPersons",0);
        if (nPersons == 0){
            printf("No people found in the training database 'facedata.xml'.\n");
            return 0;
        }
        for (i=0; i<nPersons; i++){
            string sPersonName;
            char varname[200];
            sprintf(varname,"personName_%d",(i+1));
            sPersonName = cvReadStringByName(fileStorage,0,varname);
            personNames.push_back(sPersonName);
        }

        nEigens = cvReadIntByName(fileStorage, 0, "nEigens", 0);
        nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0);
        *pTrainPersonNumMat = (CvMat *)cvReadByName(fileStorage, 0, "trainPersonNumMat", 0);
        eigenValMat  = (CvMat *)cvReadByName(fileStorage, 0, "eigenValMat", 0);
        projectedTrainFaceMat = (CvMat *)cvReadByName(fileStorage, 0, "projectedTrainFaceMat", 0);
        pAvgTrainImg = (IplImage *)cvReadByName(fileStorage, 0, "avgTrainImg", 0);
        eigenVectArr = (IplImage **)cvAlloc(nTrainFaces*sizeof(IplImage *));
        for(i=0; i<nEigens; i++)
        {
                char varname[200];
                sprintf( varname, "eigenVect_%d", i );
                eigenVectArr[i] = (IplImage *)cvReadByName(fileStorage, 0, varname, 0);
        }

        // release the file-storage interface
        cvReleaseFileStorage( &fileStorage );

        return 1;
}


//////////////////////////////////
// storeTrainingData()
//
void FaceRec::storeTrainingData()
{
        CvFileStorage * fileStorage;
        int i;

        // create a file-storage interface
        fileStorage = cvOpenFileStorage( "facedata.xml", 0, CV_STORAGE_WRITE );

        // store all the data
        cvWriteInt( fileStorage, "nEigens", nEigens );
        cvWriteInt( fileStorage, "nTrainFaces", nTrainFaces );
        cvWrite(fileStorage, "trainPersonNumMat", personNumTruthMat, cvAttrList(0,0));
        cvWrite(fileStorage, "eigenValMat", eigenValMat, cvAttrList(0,0));
        cvWrite(fileStorage, "projectedTrainFaceMat", projectedTrainFaceMat, cvAttrList(0,0));
        cvWrite(fileStorage, "avgTrainImg", pAvgTrainImg, cvAttrList(0,0));
        for(i=0; i<nEigens; i++)
        {
                char varname[200];
                sprintf( varname, "eigenVect_%d", i );
                cvWrite(fileStorage, varname, eigenVectArr[i], cvAttrList(0,0));
        }

        // release the file-storage interface
        cvReleaseFileStorage( &fileStorage );
}


//////////////////////////////////
// findNearestNeighbor()
//
int FaceRec::findNearestNeighbor(float * projectedTestFace)
{
        //double leastDistSq = 1e12;
        double leastDistSq = DBL_MAX;
        int i, iTrain, iNearest = 0;

        for(iTrain=0; iTrain<nTrainFaces; iTrain++)
        {
                double distSq=0;

                for(i=0; i<nEigens; i++)
                {
                        float d_i =
                                projectedTestFace[i] -
                                projectedTrainFaceMat->data.fl[iTrain*nEigens + i];
                 //       distSq += d_i*d_i / eigenValMat->data.fl[i];  // Mahalanobis
                        distSq += d_i*d_i; // Euclidean
                }

                if(distSq < leastDistSq)
                {
                        leastDistSq = distSq;
                        iNearest = iTrain;
                }
        }
    fConfidence = 1.0f - sqrt( leastDistSq / (float)(nTrainFaces * nEigens) ) / 255.0f;
    iCount++;
    if (fConfidence > threshold)
        recCount++;
    printf("Confidence = %f\n",fConfidence);
        return iNearest;
}


//////////////////////////////////
// doPCA()
//
void FaceRec::doPCA()
{
        int i;
        CvTermCriteria calcLimit;
        CvSize faceImgSize;

        // set the number of eigenvalues to use
        nEigens = nTrainFaces-1;

        // allocate the eigenvector images
        faceImgSize.width  = faceImgArr[0]->width;
        faceImgSize.height = faceImgArr[0]->height;
        eigenVectArr = (IplImage**)cvAlloc(sizeof(IplImage*) * nEigens);
        for(i=0; i<nEigens; i++)
                eigenVectArr[i] = cvCreateImage(faceImgSize, IPL_DEPTH_32F, 1);

        // allocate the eigenvalue array
        eigenValMat = cvCreateMat( 1, nEigens, CV_32FC1 );

        // allocate the averaged image
        pAvgTrainImg = cvCreateImage(faceImgSize, IPL_DEPTH_32F, 1);

        // set the PCA termination criterion
        calcLimit = cvTermCriteria( CV_TERMCRIT_ITER, nEigens, 1);

        // compute average image, eigenvalues, and eigenvectors
        cvCalcEigenObjects(
                nTrainFaces,
                (void*)faceImgArr,
                (void*)eigenVectArr,
                CV_EIGOBJ_NO_CALLBACK,
                0,
                0,
                &calcLimit,
                pAvgTrainImg,
                eigenValMat->data.fl);

        cvNormalize(eigenValMat, eigenValMat, 1, 0, CV_L1, 0);
}


//////////////////////////////////
// loadFaceImgArray()
//
int FaceRec::loadFaceImgArray(const char* filename)
{
        FILE * imgListFile = 0;
        char imgFilename[512];
        int iFace, nFaces=0;
        int info;


        // open the input file
        if( !(imgListFile = fopen(filename, "r")) )
        {
                fprintf(stderr, "Can\'t open file %s\n", filename);
                return 0;
        }

        // count the number of faces
        while( fgets(imgFilename, 512, imgListFile) ) ++nFaces;
        rewind(imgListFile);

        // allocate the face-image array and person number matrix
        faceImgArr        = (IplImage **)cvAlloc( nFaces*sizeof(IplImage *) );
        personNumTruthMat = cvCreateMat( 1, nFaces, CV_32SC1 );

        // store the face images in an array
        for(iFace=0; iFace<nFaces; iFace++)
        {
                // read person number and name of image file
               info = fscanf(imgListFile,
                        "%d %s", personNumTruthMat->data.i+iFace, imgFilename);
               if (info == EOF)
                       break;
                // load the face image
                faceImgArr[iFace] = cvLoadImage(imgFilename, CV_LOAD_IMAGE_GRAYSCALE);

                if( !faceImgArr[iFace] )
                {
                        fprintf(stderr, "Can\'t load image from %s\n", imgFilename);
                        return 0;
                }
        }

        fclose(imgListFile);

        return nFaces;
}

void FaceRec::unloadTrainingdata()
{
     recCount           = 0;
     iCount             = 0;
     personNumTruthMat = 0;      // array of person numbers
     nTrainFaces               = 0;      // the number of training images
     nEigens                   = 0;      // the number of eigenvalues
     pAvgTrainImg       = 0;      // the average image
     eigenVectArr      = 0;      // eigenvectors
     eigenValMat           = 0;      // eigenvalues
     projectedTrainFaceMat = 0;      // projected training faces
     trainPersonNumMat = 0;  // the person numbers during training
     projectedTestFace = 0;   // projected test faces
     threshold         = 0.85f;
}
