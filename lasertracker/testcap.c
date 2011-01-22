/*note that you need to include the /include in your path
I figure out which versions of which headers I want explicitly*/
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "stdlib.h"
#include "stdio.h"

/* this program reads from the camera 

Note: No error checkig currently exists, so make it right.  */

/*an HSV struct, useful as a typesafe tuple*/
typedef struct _HSV
{
    int hvalue;
    int svalue;
    int vvalue;
} HSV;

/*the world as passed into the callback, updates cvpoint
image is the pointer to something that changes, but it itself
is immutable. point on othe other hand is a hard side effect*/
typedef struct _world
{
    IplImage * * image;
    CvPoint point;
} World;
    


/* all of these following local functions are indempotent and
they do however primarily use return by reference for 
reasons of not having a tuple

helper function for mouse callback, gets the hsv values and populates
them in an object(hsv). Referentially transparent, data is the return
value*/
static void GetHsvFromXY(IplImage const * const target, HSV * const data,
                                int const x, int const y);

/*splits the image into the HSV and returns them by pointer
referentially transparent, splane, vplane, hplane are the return values*/
static void split
(IplImage * const hplane, IplImage * const splane, IplImage * const vplane,
IplImage const * const input);

/*mouse callback event for window, gets the HSV value and prints 
populates it, constains a hard side effect in param, updates the
world point to be the retrieved value*/
static void getHsvValue(int event, int x, int y,
    int flags, void * param);

/* splits the image up into the hsv planes
hplane, splane and vplane are all the destinations
for the data, and will be overwritten(should be null)
Datapointer is the image that will get split
(will not be modified, should be rgb) */
static void split
(IplImage * const hplane, IplImage * const splane, IplImage * const vplane,
IplImage const * const input)
{
    /*make the HSV image pointer, initialize it
    note that this may be wrong, it assumes a BGR image*/
    IplImage *  hsv = cvCreateImage(cvGetSize(input), 8, 3);
    cvCvtColor(input, hsv, CV_BGR2HSV);

    /* create the 3 images to hold the data, note they are
    one channel images*/
    /*do the split, 0 represents the last channel which is empty*/
    cvCvtPixToPlane(hsv, hplane, splane, vplane, 0);
    /* make sure to clean up all allocated memory*/
    cvReleaseImage(&hsv);
}


/*mouse callback, gets the HSV of the target
param is a pointer to the location of the frame(IplImage**)
hence it being const. The Frame may change but not in this
functions scope after it dereference(note, this is getting a pointer
to internal of the avi stream. I can't stop it from being mutated
(although I don't think it does mutate which means if pointer
assignment is atomic, this is fairly threadsafe. It's not
so this isn't really threadsafe)
POSSIBLE RACE CONDITION HERE
NEVER MIND, DEFINITE RACECONDITION HERE
Side effects: updates the value in param, which is a  world pointer.
the updated value is the current mouse x, y coordinates, if there isa
better way of reaping the values I can't see it.

Full alteration to CPS is infeasible since the callback doesn't support it.
(and it's lot of code)
*/
static void getHsvValue(int event, int x, int y,
    int flags, void * param)
{
    /*cast the param to IplImage**, dereference with *, and assign
    done const correctly*/
    World * const getcoords = 
        /*the second pointer is a true const and will not be
        raced. The other two can't be trusted
        the first star is changed by cvqueryframe
        the iplimage is changed by behind the scenes stuff
        maybe.*/
    ((World * const) (param));

    /*something to holdthe hsv image*/
    IplImage * hsv = cvCreateImage(cvGetSize((*(getcoords->image))), 8, 3);
    /*this copies, thereby avoiding...nothing*/
    cvCvtColor((*(getcoords->image)), hsv, CV_BGR2HSV);

    /*get the HSV of the coords on the image*/
    HSV * targetval = (HSV*) malloc(sizeof(HSV));

    GetHsvFromXY((*(getcoords->image)),targetval, x, y);

    /*print the HSV of the cooridnates on left button down*/
    if(event == CV_EVENT_LBUTTONDOWN)
        getcoords->point = cvPoint(x,y);
    free(targetval);
    cvReleaseImage(&hsv);
}

/*helper function for gethsvvalue callback, separated for diagnostic
reasons(should be inlined probably)*/
static void GetHsvFromXY(IplImage const * const target, HSV * const data,
                                int const x, int const y)
{

    /*get the data*/
    uchar const * const row = (uchar*)(
                target->imageData + /*imagedata*/
                /*add the offset to get to the x,y pixel, y*sizeof row
                gives the column, x* the number of channels gives the channel
                +0 is H, +1 is S, +2 is V*/
                y*target->widthStep + x * target->nChannels);
    data->hvalue = row[0];
    data->svalue = row[1];
    data->vvalue = row[2];
    
}

int main( int argc, char** argv)
{

    /*initialize the window names*/
    char const * const window = "Example";
    char const * const huewin = "HueWindow";
    char const * const satwin = "SaturationWindow";
    char const * const valwin = "ValueWindow";

    /*the capture and the current frame*/
    CvCapture* capture = NULL;
    IplImage* frame = NULL;

    /*flag*/
    char c = 0;

    /*the hsv images INITIALIZE EVERYTHING TO NULL*/
    IplImage * hueimg = NULL;
    IplImage * valimg = NULL;
    IplImage * satimg = NULL;

    World * continuation = (World *)malloc(sizeof(World));

    
    /*initialize the windows*/
    cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(huewin, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(satwin, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(valwin, CV_WINDOW_AUTOSIZE);

    
    /*captures and grabs a frame from the capture
    the capture owns the frame, so don't dealloate it*/
    capture = cvCaptureFromCAM(CV_CAP_VFW);
    frame = cvQueryFrame(capture);
    continuation->image = &frame;
    continuation->point = cvPoint(0,0);

    /*NOTE CONTINUATION CONTAINS SHARED STATE!
      point is updated asynchronously and read, this is
      a hard side effect and will have to be locked if this
      code is multithreaded*/

    /*attach my event that gets the info I desire
    as a event on the window that has the HSV values*/
    cvSetMouseCallback(window, getHsvValue, continuation);

    /*images to be built and displayed*/
    hueimg = cvCreateImage(cvGetSize(frame), 8, 1 );
    valimg = cvCreateImage(cvGetSize(frame), 8, 1 );
    satimg = cvCreateImage(cvGetSize(frame), 8, 1 );


    /*rip apart into HSV(from BGR) and display
    wait until the movie given goes black, or esc(27) is passed*/
    while(frame != NULL && c != 27)
    {

       /*split into planes*/
       split( hueimg, valimg, satimg, frame );

       /*display images in the 4 frames*/
       cvShowImage(window, frame);
       cvShowImage(huewin, hueimg);
       cvShowImage(valwin, valimg);
       cvShowImage(satwin, satimg);
       
       /*wait for 33 millisecond for the response*/
       c = cvWaitKey(33);

       /*get next frame pointer*/
       frame = cvQueryFrame(capture);

    }
    /*clean up the images before you use them again*/
    cvReleaseImage(&hueimg);
    cvReleaseImage(&valimg);
    cvReleaseImage(&satimg);

    /*delete everything*/
    cvReleaseCapture(&capture);
    cvDestroyWindow(window);
    cvDestroyWindow(huewin);
    cvDestroyWindow(valwin);
    cvDestroyWindow(satwin);
}
