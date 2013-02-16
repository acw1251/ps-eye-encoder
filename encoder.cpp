#include <iostream>
#include <opencv.hpp>

#include "arduinoUart.hpp"

using namespace std;

/*
#define BLUE_H 216/2
#define BLUE_H_MIN 185/2
#define BLUE_H_MAX 205/2
#define BLUE_S_MIN 70
#define BLUE_V_MIN 30

#define GREEN_H 120/2
#define GREEN_H_MIN 30
#define GREEN_H_MAX 50
#define GREEN_S_MIN 50
#define GREEN_V_MIN 50
*/

#define BLUE_H_MIN (210 / 2)
#define BLUE_H_MAX (255 / 2)
#define BLUE_S_MIN (25 * 5 / 2)
#define BLUE_S_MAX 255
#define BLUE_V_MIN (5 * 5 / 2)
#define BLUE_V_MAX 255

#define GREEN_H_MIN 47
#define GREEN_H_MAX 80
#define GREEN_S_MIN 91
#define GREEN_S_MAX 210
#define GREEN_V_MIN 33
#define GREEN_V_MAX 187

#define ORIGIN_H_MIN    BLUE_H_MIN
#define ORIGIN_H_MAX    BLUE_H_MAX
#define ORIGIN_S_MIN    BLUE_S_MIN
#define ORIGIN_S_MAX    BLUE_S_MAX
#define ORIGIN_V_MIN    BLUE_V_MIN
#define ORIGIN_V_MAX    BLUE_V_MAX

#define MASS_H_MIN      GREEN_H_MIN
#define MASS_H_MAX      GREEN_H_MAX
#define MASS_S_MIN      GREEN_S_MIN
#define MASS_S_MAX      GREEN_S_MAX
#define MASS_V_MIN      GREEN_V_MIN
#define MASS_V_MAX      GREEN_V_MAX

arduinoUart* arduino;

double calculateAngle( IplImage* img, IplImage** imgOrigin, IplImage** imgMass )
{
    IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
    cvCvtColor(img, imgHSV, CV_BGR2HSV);

    IplImage* tmpThresh;

    // Origin
    tmpThresh = cvCreateImage(cvGetSize(img), 8, 1);
    *imgOrigin = cvCreateImage(cvGetSize(img), 8, 1);
    cvInRangeS(imgHSV, cvScalar(ORIGIN_H_MIN, ORIGIN_S_MIN, ORIGIN_V_MIN), cvScalar(ORIGIN_H_MAX, ORIGIN_S_MAX, ORIGIN_V_MAX), tmpThresh);
    cvErode(tmpThresh, *imgOrigin, NULL, 0);
    CvMoments *originMoments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(*imgOrigin, originMoments, 1);
    double originX = cvGetSpatialMoment(originMoments, 1, 0) / cvGetCentralMoment(originMoments, 0, 0);
    double originY = cvGetSpatialMoment(originMoments, 0, 1) / cvGetCentralMoment(originMoments, 0, 0);
    cvReleaseImage(&tmpThresh);

    // Mass
    tmpThresh = cvCreateImage(cvGetSize(img), 8, 1);
    *imgMass = cvCreateImage(cvGetSize(img), 8, 1);
    cvInRangeS(imgHSV, cvScalar(MASS_H_MIN, MASS_S_MIN, MASS_V_MIN), cvScalar(MASS_H_MAX, MASS_S_MAX, MASS_V_MAX), tmpThresh);
    cvErode(tmpThresh, *imgMass, NULL, 0);
    CvMoments *massMoments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(*imgMass, massMoments, 1);
    double massX = cvGetSpatialMoment(massMoments, 1, 0) / cvGetCentralMoment(massMoments, 0, 0);
    double massY = cvGetSpatialMoment(massMoments, 0, 1) / cvGetCentralMoment(massMoments, 0, 0);
    cvReleaseImage(&tmpThresh);

    // double angle_rad = atan2( (massY - originY), (massX - originX) );
    double angle_rad = atan2( (massX - originX), -(massY - originY) );

    // Circle points on img
    cvLine( img, cvPoint((int) originX, (int) originY), cvPoint((int) originX, (int) 0), cvScalar( 0,255,255 ), 1 );
    cvCircle( img, cvPoint((int) originX, (int) originY), 10, cvScalar( 255,0,0 ), 3 );
    cvCircle( img, cvPoint((int) massX, (int) massY), 10, cvScalar( 0,255,0 ), 3 );
    cvLine( img, cvPoint((int) originX, (int) originY), cvPoint((int) massX, (int) massY), cvScalar( 0,0,255 ), 3 );

    cvReleaseImage(&imgHSV);
    delete originMoments;
    delete massMoments;

    return angle_rad * 180.0 / 3.14159;
}

bool sendAngle( double angle )
{
    int int_angle;
    int_angle = (int) ((angle / 180.0) * (128.0 * 256.0));

    cout << "Angle = " << angle << endl;
    cout << "Writing character: " << int_angle << endl;
    cout << "Arduino is getting " << (float) int_angle / (128.0 * 256.0) * 180.0 << endl;
    cout << endl;

    arduino->write_byte( (unsigned char) ((int_angle & 0x0000FF00) >> 8) );
    return arduino->write_byte( (unsigned char) (int_angle & 0x000000FF) );
}

int main()
{
    // Initialize Arduino
#ifdef __linux__
    arduino = new arduinoUart( "/dev/ttyACM0" );
#elif defined _WIN32
    arduino = new arduinoUart( "COM2" );
#else
#error "Unknown system type"
#endif

    CvCapture *webcam = NULL;
    webcam = cvCreateCameraCapture(1);
    if(webcam!=NULL) {
        cvNamedWindow("video");
        cvNamedWindow("origin and mass");
        cvNamedWindow("hue");
        cvNamedWindow("saturation");
        cvNamedWindow("value");
        IplImage* imgMass = NULL;
        IplImage* imgOrigin = NULL;
        while(1) {
            IplImage *frame = cvQueryFrame(webcam);
            cvSaveImage( "screenshot.jpg" , frame );
            double angle = calculateAngle( frame, &imgOrigin, &imgMass );
            sendAngle( angle );

            cvShowImage("video", frame);
            cvAdd(imgOrigin, imgMass, imgOrigin);
            cvShowImage("origin and mass", imgOrigin);

            IplImage* hsvImage = cvCreateImage(cvGetSize(frame), 8, 3);
            cvCvtColor(frame, hsvImage, CV_BGR2HSV);

            IplImage* hueImage = cvCreateImage(cvGetSize(frame), 8, 1);
            IplImage* satImage = cvCreateImage(cvGetSize(frame), 8, 1);
            IplImage* valImage = cvCreateImage(cvGetSize(frame), 8, 1);
            cvSplit(hsvImage, hueImage, satImage, valImage, 0);

            cvShowImage("hue", hueImage);
            cvShowImage("saturation", satImage);
            cvShowImage("value", valImage);

            if( cvWaitKey(10) != -1 ) break;

            cvReleaseImage(&imgOrigin);
            cvReleaseImage(&imgMass);
            cvReleaseImage(&hsvImage);
            cvReleaseImage(&hueImage);
            cvReleaseImage(&satImage);
            cvReleaseImage(&valImage);

            // Pauses until arduino sends 
            unsigned char tmp = 'N';
            // arduino->read( &tmp, 1 );
            // if( tmp != 'Y' ) {
            //     cout << "!!!ERROR!!!" << endl;
            // } else {
            //     cout << "Received byte from Arduino, running encoder again." << endl;
            // }
        }
        cvReleaseCapture(&webcam);
        return 0;
    } else {
        std::cout<<"CAMERA NOT DETECTED"<<std::endl;
        return 0;
    }
}

