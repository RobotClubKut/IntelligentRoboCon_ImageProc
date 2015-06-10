#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <cstdio>
#include <Windows.h>

#include "Labeling.h"			/* ラベリング処理 */
#include "BallDetect.h"			/* ボール認識処理 */
#include "IntelligentRobo.h"	/* ロボットの思考 */

using namespace std;
using namespace cv;

/* 真ん中と判断するのに用いるマージン */
#define CENTER_MARGIN_PX 6

#define COMBUF_SIZE 255

/* オプションでどれかひとつお選びいただけます */
#define OVERALL
//#define COLOR_TEST
//#define IMGPROC_TEST


void setThreshData(ThreshData& aRed, ThreshData& aBlue, ThreshData& aYellow)
{
	aRed.hueUpper = 15;
	aRed.hueLower = 170;
	aRed.satUpper = 255;
	aRed.satLower = 60;
	aRed.valUpper = 255;
	aRed.valLower = 80;

	aBlue.hueUpper = 120;
	aBlue.hueLower = 100;
	aBlue.satUpper = 255;
	aBlue.satLower = 150;
	aBlue.valUpper = 255;
	aBlue.valLower = 30;

	aYellow.hueUpper = 40;
	aYellow.hueLower = 13;
	aYellow.satUpper = 255;
	aYellow.satLower = 100;
	aYellow.valUpper = 255;
	aYellow.valLower = 50;
}

#ifdef COLOR_TEST
void colorTest(cv::Mat& aSorce, BallDetect& detect, ThreshData& aRed, ThreshData& aBlue, ThreshData& aYellow)
{
	Ball balls[32];
	int numOfBalls;
	std::vector<cv::Mat> channel;
	cv::Mat hsvImg, binRed, binBlue, binYellow;
	cv::cvtColor(aSorce, hsvImg, CV_BGR2HSV);
	cv::Point center(aSorce.cols / 2, aSorce.rows / 2);
	cv::circle(aSorce, center, 20, cv::Scalar(0, 0, 255));
	cv::split(hsvImg, channel);
	printf("%4d %4d %4d\n",
		channel[0].at<unsigned char>(center), channel[1].at<unsigned char>(center), channel[2].at<unsigned char>(center));
	detect.threshold(aSorce, binRed, aRed);
	imshow("red", binRed);
	//detect.getBallData(binRed, balls, numOfBalls);
	//chooseBall(balls, numOfBalls, x, y);

	detect.threshold(aSorce, binBlue, aBlue);
	imshow("blue", binBlue);
	detect.threshold(aSorce, binYellow, aYellow);
	imshow("yellow", binYellow);
}
#endif

#ifdef IMGPROC_TEST
void lineTraceTest(Mat& aSrc, Mat& aDst)
{
	int line1 = 0, line2 = 0;
	threshBin(aSrc, aDst, 64);
	senseLine(aSrc, aDst, line1, line2);
	int diff = line1 - line2;
	const int lineDist = LINE_CHECK_DIST_1 - LINE_CHECK_DIST_2;
	double inclination = diff != 0 ? ((double)diff / (double)lineDist) : 0;
	cout << inclination << endl;
}
#endif /* IMGPROC_TEST */

//領域外を塗りつぶす
//引数 y0, y1
cv::Mat fillOutside(int y0, int y1、cv::Mat src){
	cv::Mat result;
	if(y0 > y1){
		y0 = y1;
	}
	for(int y = 0; y < y0; ++y){
		for(int x = 0; x < src.cols; ++x){
			// 画像のチャネル数分だけループ。白黒の場合は1回、カラーの場合は3回　　　　　
			for(int c = 0; c < src.channels(); ++c){
				src.data[ y * src.step + x * src.elemSize() + c ] = 0xff;
			}
		}
	}
}

int main(int argc, const char* argv[])
{
	//cv::Mat src, img, gray, binRed, binBlue, binYellow;
	cv::Mat src, dst;

	ThreshData threshRed;
	ThreshData threshBlue;
	ThreshData threshYellow;
	setThreshData(threshRed, threshBlue, threshYellow);

	cv::VideoCapture cap(2);
	if (!cap.isOpened()){
		return -1;
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH_PX);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT_PX);

	//BallDetect detect(10, 200);

#ifdef OVERALL
	IntelligentRobo robot(threshRed, threshBlue, threshYellow);
	robot.openCon();
	cv::namedWindow("red", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("blue", cv::WINDOW_AUTOSIZE);
	cv:: namedWindow("yellow", cv::WINDOW_AUTOSIZE);
#endif /* OVERALL */
#ifdef COLOR_TEST
	namedWindow("red", WINDOW_AUTOSIZE);
	namedWindow("blue", WINDOW_AUTOSIZE);
	namedWindow("yellow", WINDOW_AUTOSIZE);
	BallDetect detect(10, 200);
#endif
	cv::namedWindow("result", cv::WINDOW_AUTOSIZE);

	int numOfRed = 0, numOfYellow = 0, numOfBlue = 0;
	while (true){
		cap >> src;
		//ここで塗りつぶす

		/*
		detect.threshold(src, binRed, threshRed);
		detect.threshold(src, binBlue, threshBlue);
		detect.threshold(src, binYellow, threshYellow);
		detect.getBallData(binRed, ballRed, numOfRed, &src);
		detect.getBallData(binBlue, ballBlue, numOfBlue, &src);
		detect.getBallData(binYellow, ballYellow, numOfYellow, &src);*/
#ifdef OVERALL
		robot.intelligence(src);
#endif /* OVERALL */
#ifdef COLOR_TEST
		colorTest(src, detect, threshRed, threshBlue, threshYellow);
#endif /* COLOR_TEST */
#ifdef IMGPROC_TEST
		lineTraceTest(src, dst);
		imshow("red", dst);
#endif /* IMGPROC_TEST */

		imshow("result", src);
		if (cv::waitKey(1) >= 0) break;
	}
	cv::destroyAllWindows();

	return 0;
}
