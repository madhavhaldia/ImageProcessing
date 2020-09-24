#include "cxcore.h"
#include "highgui.h"
#include<cv.h>
int main()
{
int i=1;
IplImage* input;
IplImage* dilate;
IplImage* erode;
IplConvKernel *structure_element;
structure_element=cvCreateStructuringElementEx(i*2+1, i*2+1,i,i,CV_SHAPE_ELLIPSE ); // Defines the structural element
cvNamedWindow("ii", 1);
cvNamedWindow("oo_dilate",1);
cvNamedWindow("oo_erode",1);
input = cvLoadImage("apple.jpg",1);
cvShowImage( "ii", input );
//make erode and dilate, clones of input (remember that cloning automatically copies height, width etc.)
dilate=cvCloneImage( input );
erode=cvCloneImage( input );
//dilate image
cvDilate(input,dilate,structure_element ,1);
//cvDilate(input image pointer , output image pointer , structural element, number of iterations)
//erode image
cvErode(input,erode,NULL,1);
//cvErode(input image pointer , output image pointer , structural element, number of iterations)
cvShowImage( "oo_dilate", dilate);
cvShowImage( "input", input);
cvShowImage( "oo_erode", erode );
cvWaitKey(0);
cvDestroyWindow( "ii" );
cvDestroyWindow( "oo_dilate" );
cvDestroyWindow( "oo_erode" );
cvReleaseImage( &input );
cvReleaseImage( &dilate );
cvReleaseImage( &erode );
return 0;
}