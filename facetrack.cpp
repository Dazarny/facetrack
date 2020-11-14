/**
 * @file objectDetection.cpp
 * Remixed by Hazim Bitar
 * Based on code writtenby A. Huaman ( based in the classic facedetect.cpp in samples/c )
 * @brief A simplified version of facedetect.cpp, show how to load a cascade classifier and how to find objects (Face + eyes) in a video stream
 * Mdified to intercept X,Y of center of face object and sends them to Arduino Uno via serial
 */
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include "tserial.h"

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
String face_cascade_name = "haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Capture - Face detection - Remixed by TechBitar";

// Serial to Arduino global declarations
int arduino_command;
Tserial* arduino_com;
short MSBLSB = 0;
unsigned char MSB = 0;
unsigned char LSB = 0;
// Serial to Arduino global declarations

int main(int argc, const char** argv)
{
    printf("\n===Face track===\n");
    //CvCapture* capture; it was changed due to open cv updates
    VideoCapture capture;
    Mat frame;

    // serial to Arduino setup 
    arduino_com = new Tserial();
    if (arduino_com != 0) {
        arduino_com->connect((char *)"COM3", 57600, spNONE);
    }
    // serial to Arduino setup 

    //-- 1. Load the cascades
    if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading \n"); return -1; };
    if (!eyes_cascade.load(eyes_cascade_name)) { printf("--(!)Error loading \n"); return -1; };

    //-- 2. Read the video stream
    //capture = cvCaptureFromCAM(3); it was changed due to open cv updates
    capture.open(0);
    //if (capture) -- it was changed due to open cv updates
    if (capture.isOpened())
    {
        while (true)
        {
            //frame = cvQueryFrame(capture); it was changed due to open cv updates
            capture.read(frame);
            //-- 3. Apply the classifier to the frame
            if (!frame.empty())
            {
                detectAndDisplay(frame);
            }
            else
            {
                printf(" --(!) No captured frame -- Break!"); break;
            }

            int c = waitKey(10);
            if ((char)c == 'c') { break; }
        }
    }
    // Serial to Arduino - shutdown
    arduino_com->disconnect();
    delete arduino_com;
    arduino_com = 0;
    // Serial to Arduino - shutdown
    return 0;
}

/**
 * @function detectAndDisplay
 */
void detectAndDisplay(Mat frame)
{
    

    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor(frame, frame_gray, /*CV_BGR2GRAY -- it was changed due to open cv updates*/cv::COLOR_BGRA2GRAY);
    equalizeHist(frame_gray, frame_gray);
    //-- Detect faces
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | /*CV_HAAR_SCALE_IMAGE -- it was changed due to open cv updates*/ CASCADE_SCALE_IMAGE, Size(30, 30));

    for (int i = 0; i < faces.size(); i++)
    {
        Point center(faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5);
        ellipse(frame, center, Size(faces[i].width * 0.5, faces[i].height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 2, 8, 0);
        cout << "X:" << faces[i].x  <<  "  y:" << faces[i].y  << endl;

         // send X,Y of face center to serial com port	
         // send X axis
         // read least significant byte 
        LSB = faces[i].x & 0xff;
        // read next significant byte 
        MSB = (faces[i].x >> 8) & 0xff;
        arduino_com->sendChar(MSB);
        arduino_com->sendChar(LSB);

        // Send Y axis
        LSB = faces[i].y & 0xff;
        MSB = (faces[i].y >> 8) & 0xff;
        arduino_com->sendChar(MSB);
        arduino_com->sendChar(LSB);
        // serial com port send	 

        //Mat faceROI = frame_gray(faces[i]);
        //std::vector<Rect> eyes;
    }

    //-- Show what you got
    imshow(window_name, frame);

}

