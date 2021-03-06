//Face recognition 
#include "stdafx.h" 
#include <stdio.h> 
#include <string.h> 
#include "cv.h" 
#include "cvaux.h" 
#include "highgui.h" 
 
//// Global variables 
IplImage ** faceImgArr = 0; // array of face images 
IplImage ** faceImgArr1 = 0; // array of face images 
CvMat * personNumTruthMat = 0; // array of person numbers 
int nTrainFaces = 0; // the number of training 
images 
int nEigens = 0; // the number of eigenvalues 
IplImage * pAvgTrainImg = 0; // the average image 
IplImage ** eigenVectArr = 0; // eigenvectors 
CvMat * eigenValMat = 0; // eigenvalues 
CvMat * projectedTrainFaceMat = 0; // projected training faces 
 
CvHaarClassifierCascade *cascade; 
CvMemStorage *storage; 
 
//// Function prototypes 
void learn(); 
void recognize(); 
void doPCA(); 
void storeTrainingData(); 
int loadTrainingData(CvMat ** pTrainPersonNumMat); 
int findNearestNeighbor(float * projectedTestFace); 
int loadFaceImgArray(char * filename); 
int loadFaceImgArray1(char * filename); 
void printUsage(); 
//void cvShowManyImages(char* title, int nArgs, ...); 
 
void detectFaces( IplImage *img ); 
IplImage *CopySubImage(IplImage *imatge,int xorigen, int yorigen,int width, int height); 81 
 
 
// Main 
int main( int argc, char** argv ) { 
	CvCapture *capture; 
	IplImage *frame; 
	int key = 1; 
	char *filename = "haarcascade_frontalface_alt.xml"; 
 
	/* load the classifier*/ 
	cascade = ( CvHaarClassifierCascade* )cvLoad( filename, 0, 0, 0 ); 
 
	/* setup memory buffer; needed by the face detector */ 
	storage = cvCreateMemStorage( 0 ); 
  
	/* initialize camera */ 
	capture = cvCaptureFromCAM( CV_CAP_ANY ); // CV_CAP_ANY 
 
	/* always check */ 
	assert( cascade && storage && capture ); 
 
	/* create a window */ 
	cvNamedWindow( "video", 1 ); 
 
	while( key != 'q' ) { 
		/* get a frame */ 
		frame = cvQueryFrame( capture ); 
 
		/* always check */ 
		if( !frame ) break; 
 
		/* detect faces and display video */ 
		detectFaces( frame ); 82 
 
		/* quit if user press 'q' */ 
		key = cvWaitKey( 10 ); 
	} 
 
	/* free memory */ 
	cvReleaseCapture( &capture ); 
	cvDestroyWindow( "video" ); 
	cvReleaseHaarClassifierCascade( &cascade ); 
	cvReleaseMemStorage( &storage );  
	return 0; 
} 
 
void detectFaces( IplImage *img ) { 
	int i; 
	int or_pointx, or_pointy; 
	// To display detected face separately 
	IplImage *Extr_face; 
	IplImage* Extr_face1 = cvCreateImage(cvSize(92, 112), IPL_DEPTH_8U, 1); 
	IplImage *Extr_face_histeq; 
	char funckey = '1'; 
	char filename[80]; 
	char imgfilename[80]; 
	int newtrain_index; 
	int newtrain_imageind; 
	CvSize image_size1; 
 
	/* detect faces */ 
	CvSeq *faces = cvHaarDetectObjects( img, cascade, storage, 1.1, 3, 0 /*CV_HAAR_DO_CANNY_PRUNNING*/, cvSize( 40, 40 ) ); 
 
	/* for each face found, draw a red box */ 
	for( i = 0 ; i < ( faces ? faces->total : 0 ) ; i++ ) { 
		CvRect *r = ( CvRect* )cvGetSeqElem( faces, i ); 
		cvRectangle( img, 
		cvPoint( r->x, r->y ), 
		cvPoint( r->x + r->width, r->y + r->height), 
		CV_RGB( 255, 0, 0 ), 1, 8, 0 ); 
		or_pointx= (int)(r->x); 
		or_pointy= (int)(r->y); 
		printf ("The co ordinates are %d, %d \n", or_pointx, or_pointy ); 
 
		// To display face 
		Extr_face = CopySubImage(img, r->x, r->y, r->width, r->height); 
		cvNamedWindow( "face_shown", 2 ); 
		cvShowImage ( "face_shown", Extr_face); 
 
		cvSaveImage("test.jpg" , Extr_face); 
 
		IplImage* newim = cvLoadImage("test.jpg", CV_LOAD_IMAGE_GRAYSCALE); 
		cvNamedWindow( "face_shown", 2 ); 
		cvShowImage ( "face_shown", newim); 
		cvResize(newim, Extr_face1, CV_INTER_CUBIC); 
		cvNamedWindow( "face_shown", 2 ); 
		cvShowImage ( "face_resize", Extr_face1); 
 
		//Histogram equalization 
		image_size1 = cvGetSize( Extr_face1 ); 
		Extr_face_histeq = cvCreateImage(cvSize(image_size1.width, image_size1.height), IPL_DEPTH_8U, 1); 
		cvEqualizeHist(Extr_face1 , Extr_face_histeq); 
		cvNamedWindow( "Histogram_equalized", 3 ); 
		cvShowImage ( "Histogram_equalized", Extr_face_histeq); 
		cvSaveImage("image.pgm",Extr_face1); 
		printf("Press 'l' to learn and 'r' to recognize\n"); 
		scanf("%c", &funckey); 
 
		if (funckey == 108) { 
			printf ("Enter new train index = "); 
			scanf ("%d", &newtrain_index); 
			printf ("Enter new train index image number = "); 
			scanf ("%d", &newtrain_imageind); 
			sprintf (filename, "train_takenew1.txt");  
			
			// Added to ask for entering DB 
			FILE * imgListFile = 0; 
			//char imgFilename[512]; 
			// open the input file 
			if( !(imgListFile = fopen(filename, "a")) { 
				fprintf(stderr, "Can\'t open file %s\n", filename); 
			} 
			fprintf (imgListFile, "\n%d s%d/%d.pgm", newtrain_index, newtrain_index, newtrain_imageind); 
			fclose(imgListFile); 
 
			sprintf(imgfilename,"s%d/%d.pgm", newtrain_index, newtrain_imageind); 
			cvSaveImage(imgfilename, Extr_face1);  
			learn(); 
		} 
 
		else if (funckey == 114) { 
			recognize(); 
		}  
 
		else { 
			printf("Invalid choice"); 
		} 
	}  
	/* display video */ 
	cvShowImage( "video", img ); 
} 
 
//Routine to extract subimage 
IplImage *CopySubImage(IplImage *imatge,int xorigen, int yorigen,int width, int height){ 
	CvRect roi; 
	IplImage *resultat; 
	roi.x = xorigen; 
	roi.y = yorigen; 
	roi.width = width; 
	roi.height = height; 
 
	// Fix the ROI as the image 
	cvSetImageROI(imatge,roi); 
 
	// Create a new image with ROI in the middle 
	resultat = cvCreateImage( cvSize(roi.width, roi.height), 
	imatge->depth, imatge->nChannels ); 
	cvCopy(imatge,resultat); 
	cvResetImageROI(imatge); 
	return resultat; 
} 
 
// Subroutine to Learn the Detected Faces
void learn() { 
	int i, offset; 
	
	// load training data 
	nTrainFaces = loadFaceImgArray("train_takenew1.txt"); 86 

	if( nTrainFaces < 2 ) { 
	fprintf(stderr,  "Need 2 or more training faces\n"  "Input file contains only %d\n", nTrainFaces); 
	return; 
	} 
 
	// do PCA on the training faces 
	doPCA(); 
 
	// project the training images onto the PCA subspace 
	projectedTrainFaceMat = cvCreateMat( nTrainFaces, nEigens,  CV_32FC1 ); 
	offset = projectedTrainFaceMat->step / sizeof(float); 
	for(i=0; i<nTrainFaces; i++) { 
		//int offset = i * nEigens; 
		cvEigenDecomposite( faceImgArr[i], nEigens, eigenVectArr, 0, 0, pAvgTrainImg, projectedTrainFaceMat->data.fl + i*offset); 
	} 
 
	// store the recognition data as an xml file 
	storeTrainingData(); 
} 

// Recognize the Detected Faces
void recognize() { 
	// Continuoulsly recognize 
 
	int i, nTestFaces = 0, nTestFacesdisp = 0; // the number of test images 
	CvMat * trainPersonNumMat = 0; // the person numbers during training 
	float * projectedTestFace = 0; 
 
	IplImage *Extr_face1; 
	IplImage *Extr_face2; 
	char filename1[80]; 
	//char filename2[80]; 
	int correct = 0; 
	static int unrec_ind = 0; 
 
	// load test images and ground truth for person number 
	nTestFaces = loadFaceImgArray("test_new.txt"); 
	printf("%d test faces loaded\n", nTestFaces); 
 
	nTestFacesdisp = loadFaceImgArray1("train_dispnew.txt"); 
	printf("%d test faces loaded\n", nTestFacesdisp); 
 
	// load the saved training data 
	if( !loadTrainingData( &trainPersonNumMat ) ) return; 
 
	// project the test images onto the PCA subspace 
	projectedTestFace = (float *)cvAlloc( nEigens*sizeof(float)); 
	for(i=0; i<nTestFaces; i++) { 
		int iNearest, nearest, truth, match_index, total; 

		// project the test image onto the PCA subspace 
		cvEigenDecomposite( 
			faceImgArr[i], 
			nEigens, 
			eigenVectArr, 
			0, 0, 
			pAvgTrainImg, 
		projectedTestFace); 

	 
		iNearest = findNearestNeighbor(projectedTestFace); 
		truth = personNumTruthMat->data.i[i]; 
		nearest = trainPersonNumMat->data.i[iNearest]; 
	 
		Extr_face1 = faceImgArr[i]; 
 
		if (iNearest != -1) {  
			printf("nearest = %d, Truth = %d iNearest = %d\n", nearest, truth, iNearest); 
			// Save images 
	 
			match_index = (nearest - 1); 
			//match_index = 0; 
	 
			Extr_face2 = faceImgArr1[match_index]; 
	 
			// Count correct faces 
			total = 400; 
			if (truth == nearest) 
				correct = correct + 1; 
	 
			// Display image 
			cvNamedWindow( "face_test", 2 ); 
			cvShowImage( "face_test", Extr_face1 ); 
	 
			cvNamedWindow( "face_recognized", 2 ); 
			cvShowImage ( "face_recognized", Extr_face2); 
	 
			while( 1 ) { 
				if( cvWaitKey( 100 ) == 27 ) break; 
			} 
	 
			printf ("Correct = %d \n", correct); 
	 
			// Clean up 
			cvDestroyWindow( "face_test" ); 
			cvDestroyWindow( "face_recognized" ); 
			cvReleaseImage( &Extr_face1 ); 89 
	 
			cvReleaseImage( &Extr_face2 ); 
		} 
		else { 
			printf("Not recognized\n"); 
			sprintf(filename1,"Not recognized/%dnot_rec.jpg", 
			unrec_ind); 
			cvSaveImage(filename1 , Extr_face1); 
	 
			unrec_ind++; 
			printf(filename1); 
			while( 1 ) { 
				if( cvWaitKey( 100 ) == 27 ) break; 
			} 
		} 
	} 
} 
 
// Load training data into program
int loadTrainingData(CvMat ** pTrainPersonNumMat) { 
	CvFileStorage * fileStorage; 
	int i; 
 
	// create a file-storage interface 
	fileStorage = cvOpenFileStorage( "facedata.xml", 0, CV_STORAGE_READ ); 
	if( !fileStorage ) { 
		fprintf(stderr, "Can't open facedata.xml\n"); 
		return 0; 
	} 
 
	nEigens = cvReadIntByName(fileStorage, 0, "nEigens", 0); 
	nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0); 90 
 
	*pTrainPersonNumMat = (CvMat *)cvReadByName(fileStorage, 0, "trainPersonNumMat", 0); 
	eigenValMat = (CvMat *)cvReadByName(fileStorage, 0,  "eigenValMat", 0); 
	projectedTrainFaceMat = (CvMat *)cvReadByName(fileStorage, 0, "projectedTrainFaceMat", 0); 
	pAvgTrainImg = (IplImage *)cvReadByName(fileStorage, 0, "avgTrainImg", 0); 
	eigenVectArr = (IplImage **)cvAlloc(nTrainFaces*sizeof(IplImage *)); 
	for(i=0; i<nEigens; i++) { 
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
void storeTrainingData() { 
	CvFileStorage * fileStorage; 
	int i; 
 
 
	// create a file-storage interface 
	fileStorage = cvOpenFileStorage( "facedata.xml", 0, CV_STORAGE_WRITE ); 
	
	// store all the data 
	cvWriteInt( fileStorage, "nEigens", nEigens ); 91 
	cvWriteInt( fileStorage, "nTrainFaces", nTrainFaces ); 
	cvWrite(fileStorage, "trainPersonNumMat", personNumTruthMat, 
	cvAttrList(0,0)); 
	cvWrite(fileStorage, "eigenValMat", eigenValMat, 
	cvAttrList(0,0)); 
	cvWrite(fileStorage, "projectedTrainFaceMat", 
	projectedTrainFaceMat, cvAttrList(0,0)); 
	cvWrite(fileStorage, "avgTrainImg", pAvgTrainImg, 
	cvAttrList(0,0)); 
	for(i=0; i<nEigens; i++) { 
		char varname[200]; 
		sprintf( varname, "eigenVect_%d", i ); 
		cvWrite(fileStorage, varname, eigenVectArr[i], 
		cvAttrList(0,0)); 
	} 
 
	// release the file-storage interface 
	cvReleaseFileStorage( &fileStorage ); 
} 
 
////////////////////////////////// 
// findNearestNeighbor() 
// 
int findNearestNeighbor(float * projectedTestFace) { 
	//double leastDistSq = 1e12; 
	double leastDistSq = DBL_MAX; 
	int i, iTrain, iNearest = 0; 
 
	for(iTrain=0; iTrain<nTrainFaces; iTrain++) { 
		double distSq=0; 
 
		for(i=0; i<nEigens; i++) { 
			float d_i = projectedTestFace[i] - 
			projectedTrainFaceMat->data.fl[iTrain*nEigens+i]; 
 
			//distSq += d_i*d_i / eigenValMat->data.fl[i]; 
			distSq += d_i*d_i; // Euclidean 
		} 
 
		if(distSq < leastDistSq) { 
			leastDistSq = distSq; 
			iNearest = iTrain; 
		} 
	} 
	// Display nearest distance 
	printf ("Least distance = %lf \n", leastDistSq); 
 
	if (leastDistSq > 10000000) 
		iNearest = -1; 
 
	return iNearest; 
} 
 
////////////////////////////////// 
// doPCA() 
// 
void doPCA() { 
	int i; 
	CvTermCriteria calcLimit; 
	CvSize faceImgSize; 
 
	// set the number of eigenvalues to use 
	nEigens = nTrainFaces-1; 
 
	// allocate the eigenvector images 
	faceImgSize.width = faceImgArr[0]->width; 
	faceImgSize.height = faceImgArr[0]->height; 
	eigenVectArr = (IplImage**)cvAlloc(sizeof(IplImage*) *nEigens); 
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
int loadFaceImgArray(char * filename) { 
	FILE * imgListFile = 0; 
	char imgFilename[512]; 
	int iFace, nFaces=0; 
 
	//IplImage *Extr_face; 
	// open the input file 
	if( !(imgListFile = fopen(filename, "r")) ) { 
		fprintf(stderr, "Can\'t open file %s\n", filename); 
		return 0; 
	} 
 
	// count the number of faces 
	while( fgets(imgFilename, 512, imgListFile) ) ++nFaces; 
	rewind(imgListFile); 
 
	// allocate the face-image array and person number matrix 
	faceImgArr = (IplImage **)cvAlloc( nFaces*sizeof(IplImage *) ); 
	personNumTruthMat = cvCreateMat( 1, nFaces, CV_32SC1 ); 
 
	// store the face images in an array 
	for(iFace=0; iFace<nFaces; iFace++) { 
		// read person number and name of image file 
		fscanf(imgListFile,"%d %s", personNumTruthMat->data.i+iFace,imgFilename); 
 
		// load the face image 
		faceImgArr[iFace] = cvLoadImage(imgFilename, CV_LOAD_IMAGE_GRAYSCALE); 
 
		if( !faceImgArr[iFace] ) { 
			fprintf(stderr, "Can\'t load image from %s\n", imgFilename); 
			return 0; 
		} 
	}  
	fclose(imgListFile); 
	return nFaces; 
} 
 
// New array for displaying the recognized train image 
int loadFaceImgArray1(char * filename) { 
	FILE * imgListFile = 0; 
	char imgFilename[512]; 
	int iFace, nFaces=0; 
 
	//IplImage *Extr_face; 
	// open the input file 
	if( !(imgListFile = fopen(filename, "r")) ) { 
		fprintf(stderr, "Can\'t open file %s\n", filename); 
		return 0; 
	} 
 
	// count the number of faces 
	while( fgets(imgFilename, 512, imgListFile) ) ++nFaces; 
	rewind(imgListFile); 
 
	// allocate the face-image array and person number matrix 
	faceImgArr1 = (IplImage **)cvAlloc( nFaces*sizeof(IplImage *) ); 
	personNumTruthMat = cvCreateMat( 1, nFaces, CV_32SC1 ); 
 
	// store the face images in an array 
	for(iFace=0; iFace<nFaces; iFace++) { 
		// read person number and name of image file 
		fscanf(imgListFile,"%d %s", personNumTruthMat->data.i+iFace, imgFilename); 
 
		// load the face image 
		faceImgArr1[iFace] = cvLoadImage(imgFilename, CV_LOAD_IMAGE_GRAYSCALE); 
 
		if( !faceImgArr1[iFace] ) { 
			fprintf(stderr, "Can\'t load image from %s\n", imgFilename); 
			return 0; 
		} 
	} 
 
	fclose(imgListFile); 
	return nFaces; 
}