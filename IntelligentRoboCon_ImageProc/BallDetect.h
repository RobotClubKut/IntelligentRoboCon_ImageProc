#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <cstdio>
#include <Windows.h>
#include "Labeling.h"

using namespace cv;

/* PIXELES_THRESH以下の画素の塊をはじく */
#define DEFAULT_PIXELES_THRESH 300


struct BallData
{
	unsigned int x, y;
	unsigned int diameter;
	unsigned int numOfPixels;
	unsigned int minX, minY;
	unsigned int maxX, maxY;
};


struct ThreshData{
	unsigned char hueUpper;
	unsigned char hueLower;
	unsigned char satUpper;
	unsigned char satLower;
	unsigned char valUpper;
	unsigned char valLower;
};

class BallDetect
{
public:
	BallDetect(int aLimitOfBall);
	BallDetect(int aLimitOfBall, int aAreaPixelThresh);
	~BallDetect();
	void getBallData(cv::Mat& aSorce, BallData aResult[], int& aNumOfBall);
	void getBallData(cv::Mat& aSorce, BallData aResult[], int& aNumOfBall, cv::Mat& aResultImg);
	void threshold(const cv::Mat& aSorce, cv::Mat& aDest, const ThreshData& thresh);
	void setAreaPixelThresh(const int aValue);
private:
	int mAreaPixelThresh;
	int mLimitOfBall;
	cv::Mat *mResultImg;
};


