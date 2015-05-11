#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <cstdio>
#include <Windows.h>

#include "Labeling.h"			/* ラベリング処理 */
#include "BallDetect.h"			/* ボール認識処理 */
#include "IntelligentRobo.h"	/* ロボットの思考 */

/* 真ん中と判断するのに用いるマージン */
#define CENTER_MARGIN_PX 6

#define COMBUF_SIZE 255


void colorTest(cv::Mat& aSorce)
{
	std::vector<cv::Mat> channel;
	cv::Mat hsvImg, hue1, hue2, hue, saturation1, saturation2, saturation, value1, value2, value, hue_saturation;
	cv::cvtColor(aSorce, hsvImg, CV_BGR2HSV);
	cv::Point center(aSorce.cols / 2, aSorce.rows / 2);
	cv::circle(aSorce, center, 20, cv::Scalar(0, 0, 255));
	cv::split(hsvImg, channel);
	printf("%4d %4d %4d\n", 
		channel[0].at<unsigned char>(center), channel[1].at<unsigned char>(center), channel[2].at<unsigned char>(center));
}

int main(int argc, const char* argv[])
{
	cv::Mat src, img, gray, binRed, binBlue, binYellow;

	cv::VideoCapture cap(0);
	if (!cap.isOpened()){
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH_PX);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT_PX);

	BallData ballRed[10];
	BallData ballBlue[10];
	BallData ballYellow[10];
	ThreshData threshRed = { 0 };
	threshRed.hueUpper = 6;
	threshRed.hueLower = 170;
	threshRed.satUpper = 255;
	threshRed.satLower = 60;
	threshRed.valUpper = 255;
	threshRed.valLower = 80;
	ThreshData threshBlue = { 0 };
	threshBlue.hueUpper = 120;
	threshBlue.hueLower = 100;
	threshBlue.satUpper = 255;
	threshBlue.satLower = 160;
	threshBlue.valUpper = 255;
	threshBlue.valLower = 50;
	ThreshData threshYellow = { 0 };
	threshYellow.hueUpper = 40;
	threshYellow.hueLower = 13;
	threshYellow.satUpper = 220;
	threshYellow.satLower = 50;
	threshYellow.valUpper = 255;
	threshYellow.valLower = 50;

	BallDetect detect(10, 200);

	cv::namedWindow("result", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("red", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("blue", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("yellow", cv::WINDOW_AUTOSIZE);

	IntelligentRobo robot;

	int numOfRed = 0, numOfYellow = 0, numOfBlue = 0;
	while (true){
		cap >> src;
		detect.threshold(src, binRed, threshRed);
		detect.threshold(src, binBlue, threshBlue);
		detect.threshold(src, binYellow, threshYellow);
		detect.getBallData(binRed, ballRed, numOfRed, &src);
		detect.getBallData(binBlue, ballBlue, numOfBlue, &src);
		detect.getBallData(binYellow, ballYellow, numOfYellow, &src);
		
		// 各色から一番大きいのを選ぶ
		BallData *maxBall = NULL;
		if (numOfRed) { maxBall = ballRed; }
		if (numOfBlue && maxBall != NULL)
		{
			if (maxBall->numOfPixels < ballBlue[0].numOfPixels)
			{
				maxBall = ballBlue;
			}
		}
		if (numOfYellow && maxBall != NULL)
		{
			if (maxBall->numOfPixels < ballYellow[0].numOfPixels)
			{
				maxBall = ballYellow;
			}
		}
		
		robot.intelligence(maxBall, (maxBall != NULL) ? 1:0);
		//colorTest(src);
		robot.drawMark(src);
		cv::imshow("result", src);
		cv::imshow("red", binRed);
		cv::imshow("blue", binBlue);
		//cv::imshow("yellow", binYellow);
		if (cv::waitKey(1) >= 0) break;
	}
	cv::destroyAllWindows();

	return 0;
}