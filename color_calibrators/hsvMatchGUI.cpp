#include <iostream>
#include <opencv.hpp>

using namespace std;

int main()
{
    CvCapture *webcam = NULL;
    webcam = cvCreateCameraCapture(1);
    if(webcam != NULL) {
        cvNamedWindow("video");
        cvNamedWindow("threshold");
        cvNamedWindow("hue match");
        cvNamedWindow("saturation match");
        cvNamedWindow("value match");

        cvMoveWindow("video",100,50);
        cvMoveWindow("threshold",900,50);
        cvMoveWindow("hue match",100,350);
        cvMoveWindow("saturation match",500,350);
        cvMoveWindow("value match",900,350);

        int H_Min = 0;
        int H_Max = 179;
        int S_Min = 0;
        int S_Max = 255;
        int V_Min = 0;
        int V_Max = 255;

        cvCreateTrackbar("HueMin", "hue match", &H_Min, 179, NULL);
        cvCreateTrackbar("HueMax", "hue match", &H_Max, 179, NULL);
        cvCreateTrackbar("SatMin", "saturation match", &S_Min, 255, NULL);
        cvCreateTrackbar("SatMax", "saturation match", &S_Max, 255, NULL);
        cvCreateTrackbar("ValMin", "value match", &V_Min, 255, NULL);
        cvCreateTrackbar("ValMax", "value match", &V_Max, 255, NULL);

        while(1) {
            IplImage *frame, *imgHSV, *tmpThresh, *imgThresh, *hueImage, *hueMatch, *satImage, *satMatch, *valImage, *valMatch;
            frame = cvQueryFrame(webcam);
            cvSaveImage( "screenshot.jpg" , frame );
            // frame = cvLoadImage("screenshot.jpg");
            // Convert to HSV
            imgHSV = cvCreateImage(cvGetSize(frame), 8, 3);
            cvCvtColor(frame, imgHSV, CV_BGR2HSV);

            // Threshold and Filter
            tmpThresh = cvCreateImage(cvGetSize(frame), 8, 1);
            imgThresh = cvCreateImage(cvGetSize(frame), 8, 1);
            cvInRangeS(imgHSV, cvScalar(H_Min, S_Min, V_Min), cvScalar(H_Max, S_Max, V_Max), tmpThresh);
            cvErode(tmpThresh, imgThresh, NULL, 0);

            // Split hue, sat, and val
            // Threshold for the individual channels
            hueImage = cvCreateImage(cvGetSize(frame), 8, 1);
            hueMatch = cvCreateImage(cvGetSize(frame), 8, 1);
            satImage = cvCreateImage(cvGetSize(frame), 8, 1);
            satMatch = cvCreateImage(cvGetSize(frame), 8, 1);
            valImage = cvCreateImage(cvGetSize(frame), 8, 1);
            valMatch = cvCreateImage(cvGetSize(frame), 8, 1);
            cvSplit(imgHSV, hueImage, satImage, valImage, 0);
            cvInRangeS(hueImage, cvScalar(H_Min), cvScalar(H_Max), hueMatch);
            cvInRangeS(satImage, cvScalar(S_Min), cvScalar(S_Max), satMatch);
            cvInRangeS(valImage, cvScalar(V_Min), cvScalar(V_Max), valMatch);
            IplImage *hueMatchColor;
            hueMatchColor = cvCreateImage(cvGetSize(frame), 8, 1);
            cvMul(hueImage, hueMatch, hueMatchColor, 1/255.0);


            cvShowImage("video", frame);
            cvShowImage("threshold", imgThresh);
            cvShowImage("hue match", hueMatchColor);
            cvShowImage("saturation match", satMatch);
            cvShowImage("value match", valMatch);

            if( cvWaitKey(10) != -1 ) {
                cvSaveImage( "screenshot.jpg" , frame );
                break;
            }

            //cvReleaseImage(&frame);
            cvReleaseImage(&imgHSV);
            cvReleaseImage(&tmpThresh);
            cvReleaseImage(&imgThresh);
            cvReleaseImage(&hueImage);
            cvReleaseImage(&hueMatch);
            cvReleaseImage(&satImage);
            cvReleaseImage(&satMatch);
            cvReleaseImage(&valImage);
            cvReleaseImage(&valMatch);
        }
        cvReleaseCapture(&webcam);
        cout << "H_Min: " << H_Min << ", S_Min: " << S_Min << ", V_Min: " << V_Min << endl;
        cout << "H_Max: " << H_Max << ", S_Max: " << S_Max << ", V_Max: " << V_Max << endl;
        return 0;
    } else {
        cout << "CAMERA NOT DETECTED" << endl;
        return 0;
    }
}
