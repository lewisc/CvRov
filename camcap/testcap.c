#include "opencv/highgui.h"
#include "opencv/cv.h"

int main( int argc, char** argv)
{
    char* window = "Example";
    CvCapture* capture;
    char c = 0;
    cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
    IplImage* frame;
    
    
    if(argc == 1)
    {
        capture = cvCreateCameraCapture(-1);
    }
    else
    {
        capture = cvCreateFileCapture(argv[1]);
    }

    frame = cvQueryFrame(capture);
    while(frame != NULL && c != 27)
    {
        cvShowImage(window, frame);
        c = cvWaitKey(33);
        frame = cvQueryFrame(capture);
    }

    cvReleaseCapture(&capture);
    cvDestroyWindow(window);
}
