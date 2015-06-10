#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include <cstdio>
#include <iostream>
#include "MyTypedef.h"
#include "IntelligentRobo.h"

using namespace std;
using namespace cv;


typedef unsigned char uint8;

const int g_lineDataLength = 2;
int g_line1Data[g_lineDataLength] = { 0 };
int g_line2Data[g_lineDataLength] = { 0 };

void drawSensingLine(Mat& aSrc)
{
	int center_x = aSrc.cols / 2;
	line(aSrc,
		Point(center_x - (LINE_CHECK_WIDTH / 2), LINE_CHECK_DIST_1),
		Point(center_x + (LINE_CHECK_WIDTH / 2), LINE_CHECK_DIST_1),
		Scalar(0, 0, 255));
	line(aSrc,
		Point(center_x - (LINE_CHECK_WIDTH / 2), LINE_CHECK_DIST_2),
		Point(center_x + (LINE_CHECK_WIDTH / 2), LINE_CHECK_DIST_2),
		Scalar(0, 0, 255));
}

void threshBin(const Mat aSrc, Mat& aDst, int thresh)
{
	Mat gray;
	cvtColor(aSrc, gray, CV_BGR2GRAY, 0);
	threshold(gray, aDst, thresh, 255, THRESH_BINARY_INV);
}

//void senseLine(Mat& aSrc, Mat& aBinImg, int& line1, int& line2, int& line3, int& line4)
void senseLine(Mat& aSrc, Mat& aBinImg, int& line1, int& line2)
{
	static int c = 0;
	int center_x = aSrc.cols / 2;
	//threshBin(aSrc, aBinImg, 127);
	int x, y;
	int loop_limit = center_x + (LINE_CHECK_WIDTH / 2);
	const int firstPoint = -(LINE_CHECK_WIDTH / 2);
	int totalPoint;
	int addingPoint;
	int pixelCounter;
	int total;
	//int line1, line2;

	for (x = center_x - (LINE_CHECK_WIDTH / 2),
		y = LINE_CHECK_DIST_1,
		totalPoint = 0,
		addingPoint = firstPoint,
		pixelCounter = 0;
	x < loop_limit; x++, addingPoint++)
	{
		if (aBinImg.at<char>(y, x) != 0)
		{
			totalPoint += addingPoint;
			pixelCounter++;
		}
	}
	g_line1Data[c] = totalPoint != 0 ? totalPoint / pixelCounter : 0;
	//if (pixelCounter == 0) line1 = INT_MAX;
	/* g_lineDataLength•ª‚Ì•½‹Ï‚ð‚Æ‚é*/
	total = 0;
	for (int i = 0; i < g_lineDataLength; i++)
	{
		total += g_line1Data[i];
	}
	line1 = total / g_lineDataLength;

	for (x = center_x - (LINE_CHECK_WIDTH / 2),
		y = LINE_CHECK_DIST_2,
		totalPoint = 0,
		addingPoint = firstPoint,
		pixelCounter = 0;
	x < loop_limit; x++, addingPoint++)
	{
		if (aBinImg.at<char>(y, x) != 0)
		{
			totalPoint += addingPoint;
			pixelCounter++;
		}
	}
	g_line2Data[c] = totalPoint != 0 ? totalPoint / pixelCounter : 0;
	//if (pixelCounter == 0) line2 = INT_MAX;
	total = 0;
	for (int i = 0; i < g_lineDataLength; i++)
	{
		total += g_line2Data[i];
	}
	line2 = total / g_lineDataLength;

	c++;
	c %= g_lineDataLength;

	drawSensingLine(aSrc);
}


void senseLine2(Mat& aSrc, Mat& aBinImg, int *aLine, int aNum, int aSpace)
{
	int x, y, i;
	int w = (aNum -1) * aSpace;
	int count;
	const int y1 = 50, y2 = 120;
	//threshBin(aSrc, aBinImg, 127);
	for (x = 160 - (w / 2), i = 0; i < aNum; i++, x += aSpace)
	{
		count = 0;
		for (y = y1; y < y2; y++)
		{
			if (aBinImg.at<char>(y, x) != 0) count++;
		}
		aLine[i] = count;
		line(aSrc,
			Point(x, y1),
			Point(x, y2),
			Scalar(0, 0, 255));
		//cout << count << " , ";
	}
	//cout << endl;
}

int senseCurveLine(Mat& aSrc, Mat& aBinImg)
{
	const int th = 1;
	static int count = 0;
	static int old = 0;
	const int numOfLines = 6;
	int lines[numOfLines];
	threshBin(aSrc, aBinImg, 127);
	senseLine2(aSrc, aBinImg, lines, numOfLines, 20);
	//int l = lines[0] + lines[1] + lines[2];
	//int r = lines[3] + lines[4] + lines[5];
	int l = 0, r = 0;
	if (lines[0] >= th) l++;
	if (lines[1] >= th) l++;
	if (lines[2] >= th) l++;
	if (lines[3] >= th) r++;
	if (lines[4] >= th) r++;
	if (lines[5] >= th) r++;
	if (r - 1 > l && l == 0)
	{
		if (old == 1)
		{
			count++;
		}
		else{
			old = 1;
			count = 1;
		}
	}
	else if (l - 1 > r && r == 0)
	{
		if (old == 2)
		{
			count++;
		}
		else{
			old = 2;
			count = 1;
		}
	}
	else{
		old = count = 0;
	}

	//cout << r << " " << l << endl;
	
	if (count >= 3)
	{
		if (old == 1)
		{
			cout << "RIGHT" << endl;
		}
		else if (old == 2)
		{
			cout << "LEFT" << endl;
		}
		count = 0;
		return old;
	}
	
	return 0;
}

bool senseLine3(Mat& aSrc, Mat& aBinImg, int& line1, int& line2)
{
	const static int uy = LINE_CHECK_DIST_1 - 70;
	const static int x1 = (aSrc.cols / 2) - (LINE_CHECK_WIDTH / 2);
	const static int x2 = (aSrc.cols / 2) + (LINE_CHECK_WIDTH / 2);

	static int count = 0;

	//int line1 = 0;
	//int line2 = 0;
	line1 = 0;
	line2 = 0;

	for (int y = uy; y <= LINE_CHECK_DIST_1; y++)
	{
		if (aBinImg.at<char>(y, x1) != 0)
		{
			line1++;
			//line1 += y;
		}
		if (aBinImg.at<char>(y, x2) != 0)
		{
			line2++;
			//line2 += y;
		}
	}
	line(aSrc,
		Point(x1, LINE_CHECK_DIST_1),
		Point(x1, uy),
		Scalar(0, 0, 255));
	line(aSrc,
		Point(x2, LINE_CHECK_DIST_1),
		Point(x2, uy),
		Scalar(0, 0, 255));

	if (line1 != 0 && line2 != 0)
	{
		count++;
	}
	else{
		count = 0;
	}
	cout << "senseLine3" << endl;
	if (count >= 3)
	{
		return true;
	}
	return false;
}

/*void lineTrace(Mat& aSrc, Mat& aBinImg)
{
	
}*/