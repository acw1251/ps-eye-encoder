#include <iostream>
#include <opencv.hpp>

using namespace std;

#define BLUE_H_MIN (210 / 2)
#define BLUE_H_MAX (255 / 2)
#define BLUE_S_MIN (25 * 5 / 2)
#define BLUE_S_MAX (75 * 5 / 2)
#define BLUE_V_MIN (5 * 5 / 2)
#define BLUE_V_MAX (95 * 5 / 2)

#define GREEN_H_MIN (95 / 2)
#define GREEN_H_MAX (160 / 2)
#define GREEN_S_MIN (45 * 5 / 2)
#define GREEN_S_MAX (75 * 5 / 2)
#define GREEN_V_MIN (5 * 5 / 2)
#define GREEN_V_MAX (75 * 5 / 2)

int main()
{
    CvCapture *webcam = NULL;
    webcam = cvCreateCameraCapture(1);
    if(webcam!=NULL) {
        cvNamedWindow("video");
        cvNamedWindow("threshold");
        cvNamedWindow("hue");
        cvNamedWindow("saturation");
        cvNamedWindow("value");

        int H_Min = GREEN_H_MIN;
        int H_Max = GREEN_H_MAX;
        int S_Min = GREEN_S_MIN;
        int S_Max = GREEN_S_MAX;
        int V_Min = GREEN_V_MIN;
        int V_Max = GREEN_V_MAX;

        cvCreateTrackbar("HueMin", "threshold", &H_Min, 127, NULL);
        cvCreateTrackbar("HueMax", "threshold", &H_Max, 127, NULL);
        cvCreateTrackbar("SatMin", "threshold", &S_Min, 255, NULL);
        cvCreateTrackbar("SatMax", "threshold", &S_Max, 255, NULL);
        cvCreateTrackbar("ValMin", "threshold", &V_Min, 255, NULL);
        cvCreateTrackbar("ValMax", "threshold", &V_Max, 255, NULL);

        while(1) {
            IplImage *frame = cvQueryFrame(webcam);
            cvSaveImage( "screenshot.jpg" , frame );

            IplImage* imgHSV = cvCreateImage(cvGetSize(frame), 8, 3);
            cvCvtColor(frame, imgHSV, CV_BGR2HSV);

            IplImage* tmpThresh = cvCreateImage(cvGetSize(frame), 8, 1);
            IplImage* imgThresh = cvCreateImage(cvGetSize(frame), 8, 1);
            cvInRangeS(imgHSV, cvScalar(H_Min, S_Min, V_Min), cvScalar(H_Max, S_Max, V_Max), tmpThresh);
            cvErode(tmpThresh, imgThresh, NULL, 0);

            cvReleaseImage(&tmpThresh);

            cvShowImage("video", frame);
            cvShowImage("threshold", imgThresh);

            IplImage* hueImage = cvCreateImage(cvGetSize(frame), 8, 1);
            IplImage* satImage = cvCreateImage(cvGetSize(frame), 8, 1);
            IplImage* valImage = cvCreateImage(cvGetSize(frame), 8, 1);
            cvSplit(imgHSV, hueImage, satImage, valImage, 0);

            cvShowImage("hue", hueImage);
            cvShowImage("saturation", satImage);
            cvShowImage("value", valImage);

            if( cvWaitKey(10) != -1 ) break;

            cvReleaseImage(&imgHSV);
            cvReleaseImage(&hueImage);
            cvReleaseImage(&satImage);
            cvReleaseImage(&valImage);

        }
        cvReleaseCapture(&webcam);
        cout << "H_Min: " << H_Min << ", S_Min: " << S_Min << ", V_Min: " << V_Min << endl;
        cout << "H_Max: " << H_Max << ", S_Max: " << S_Max << ", V_Max: " << V_Max << endl;
        return 0;
    } else {
        std::cout<<"CAMERA NOT DETECTED"<<std::endl;
        return 0;
    }
}
