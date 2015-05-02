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


/*
 * 赤青黄の各色の閾値設定
 */
#define RED_HUE_THRESH_UPPER 6
#define RED_HUE_THRESH_LOWER 170
#define RED_SATURATION_THRESH 60
#define RED_VALUE_THRESH 80

#define BLUE_HUE_THRESH_UPPER 120
#define BLUE_HUE_THRESH_LOWER 100
#define BLUE_SATURATION_THRESH_UPPER 255
#define BLUE_SATURATION_THRESH_LOWER 245
#define BLUE_VALUE_THRESH_UPPER 255
#define BLUE_VALUE_THRESH_LOWER 0

#define YELLOW_HUE_THRESH_UPPER 35
#define YELLOW_HUE_THRESH_LOWER 20
#define YELLOW_SATURATION_THRESH_UPPER 190
#define YELLOW_SATURATION_THRESH_LOWER 40
#define YELLOW_VALUE_THRESH_UPPER 255
#define YELLOW_VALUE_THRESH_LOWER 110



#define COMBUF_SIZE 255


void cvtWindow2RobotView(int& ax, int& ay)
{
	static int CENTER_X = CAMERA_WIDTH_PX / 2;
	static int CENTER_Y = CAMERA_HEIGHT_PX / 2;

	ax -= CENTER_X;
	ay -= CENTER_Y;
}


void intelligence(const BallData aBalls[], int aNumOfBalls)
{
	static int sequence = 0;

	static const int CENTER_X = CAMERA_WIDTH_PX / 2;
	static const int CENTER_Y = CAMERA_HEIGHT_PX / 2;
	static const int RANGE_LOWER = CENTER_X - (CENTER_MARGIN_PX / 2);
	static const int RANGE_UPPER = CENTER_X + (CENTER_MARGIN_PX / 2);

	int targetX = aBalls[0].x;
	int targetY = aBalls[0].y;
	cvtWindow2RobotView(targetX, targetY);

	if (aNumOfBalls > 0 && abs(targetX))
	{
		// 正面にボールがある
	}
}


int main(int argc, const char* argv[])
{
	cv::Mat src, img, gray, binData;

	//Robot::SerialCom com("COM3", 9600);
	//char comBuf[COMBUF_SIZE];

	cv::VideoCapture cap(0);
	if (!cap.isOpened()){
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH_PX);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT_PX);

	BallData ball[10];
	ThreshData threshRed = { 0 };
	threshRed.hueUpper = RED_HUE_THRESH_UPPER;
	threshRed.hueLower = RED_HUE_THRESH_LOWER;
	threshRed.satUpper = 255;
	threshRed.satLower = RED_SATURATION_THRESH;
	threshRed.valUpper = 255;
	threshRed.valLower = RED_VALUE_THRESH;
	ThreshData threshBlue = { 0 };
	threshBlue.hueUpper = BLUE_HUE_THRESH_UPPER;
	threshBlue.hueLower = BLUE_HUE_THRESH_LOWER;
	threshBlue.satUpper = BLUE_SATURATION_THRESH_UPPER;
	threshBlue.satLower = BLUE_SATURATION_THRESH_LOWER;
	threshBlue.valUpper = BLUE_VALUE_THRESH_UPPER;
	threshBlue.valLower = BLUE_VALUE_THRESH_LOWER;

	BallDetect detect(10, 200);

	cv::namedWindow("result", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("red", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("blue", cv::WINDOW_AUTOSIZE);
	//cv::namedWindow("yellow", cv::WINDOW_AUTOSIZE);

	IntelligentRobo robot;

	int numOfBalls = 0;
	while (true){
		cap >> src;
		
		detect.threshold(src, binData, threshRed);
		detect.getBallData(binData, ball, numOfBalls, &src);
		//printf("ok\n");
		robot.intelligence(ball, numOfBalls);
		cv::imshow("result", src);
		cv::imshow("red", binData);
		if (cv::waitKey(1) >= 0) break;
	}
	cv::destroyAllWindows();

	return 0;
}