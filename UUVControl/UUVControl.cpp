// UUVControl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <windows.h>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	//const std::string videoStreamAddress = "http://192.168.254.1:8090/?action=stream";
	//VideoCapture cap(videoStreamAddress);	// input of video. currently file name//// in the src directory.

	VideoCapture cap(0); //capture the video from webcam

	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	cout << "Width = " << dWidth << "\n";
	cout << "Height = " << dHeight << "\n";

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 170;
	int iHighH = 179;

	int iLowS = 150;
	int iHighS = 255;

	int iLowV = 60;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);;


	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video



		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

																									  //morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
		if (dArea > 10000)
		{
			//calculate the position of the ball
			double posX = dM10 / dArea;
			double posY = dM01 / dArea;
			cout << "posX= " << posX << "\n";
			cout << "posY= " << posY << "\n";

			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
			{
				//Draw a red line from the previous point to the current point
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
			}
			

			iLastX = posX;
			iLastY = posY;
			int leftRight = 100;
			int up = 150;
			int down = 50;

			/*
			Camera Sections
			-------
			|1|2|3|
			-------
			|4|5|6|
			-------
			|7|8|9|
			-------
			*/

			if (posX < dWidth / 3 && posY < dHeight / 3) {																	//Moves UpLeft when in section 1 of Camera
				cout << "UpLeft\n";
				//move(down, 17);//move up
				//move(leftRight, 37);//move left
			}

			else if (posX > dWidth / 3 && posX < dWidth * 2 / 3 && posY < dHeight / 3) {									//Moves Up when in section 2 of Camera
				cout << "Up\n";
				//move(up, 16);//move up
			}

			else if (posX > dWidth * 2 / 3 && posY < dHeight / 3) {															//Moves UpRight when in section 3 of Camera
				cout << "UpRight\n";
				//move(leftRight, 39); //move right
				//move(up, 16);//move up

			}

			else if (posX < dWidth / 3 && posY > dHeight / 3 &&	posY < dHeight * 2 / 3) {									//Moves Left when in section 4 of Camera
				cout << "Left\n";
				//move(leftRight, 37);//move left
			}

			else if (posX > dWidth / 3 && posX < dWidth * 2 / 3  && posY > dHeight / 3 && posY < dHeight * 2 / 3) {			//Moves Straight when in section 5 of Camera
				cout << "Straight\n";

			}
			
			else if (posX > dWidth * 2 / 3 && posY > dHeight / 3 && posY < dHeight * 2 / 3) {								//Moves Right when in section 6 of Camera
				cout << "Right\n";
				//move(leftRight, 39); //move right
			}

			else if (posX < dWidth / 3 && posY > dHeight * 2 / 3) {															//Moves DownLeft when in section 7 of Camera
				cout << "DownLeft\n";

			}

			else if (posX > dWidth / 3 && posX < dWidth * 2 / 3 && posY > dHeight * 2 / 3) {								//Moves Down when in section 8 of Camera
				cout << "Down\n";
				//move(down, 17);//move down
			}
			
			else if (posX > dWidth * 2 / 3 && posY > dHeight * 2 / 3) {														//Moves DownRight when in section 9 of Camera
				cout << "DownRight\n";
				//move(leftRight, 39); //move right
				//move(down, 17);//move down

			}
		}

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //show the original image

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}

/*
void move(int time, int button1) {
	//https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx -> keycodes
	//Press right arrow 27
	INPUT ip;
	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	ip.ki.wVk = button1;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));
	Sleep(time / 2);
	//Release right arrow
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));

	//Press down arrow 25
}
*/