#include "BallDetect.h"

BallDetect::BallDetect(int aLimitOfBall)
{
	mLimitOfBall = aLimitOfBall;
	mAreaPixelThresh = DEFAULT_PIXELES_THRESH;
}


BallDetect::BallDetect(int aLimitOfBall, int aAreaPixelThresh)
{
	mLimitOfBall = aLimitOfBall;
	mAreaPixelThresh = aAreaPixelThresh;
}


BallDetect::~BallDetect()
{

}

/*void BallDetect::getBallData(cv::Mat& aSorce, BallData aResult[], int& aNumOfBalls)
{
	BallDetect::getBallData(aSorce, aResult, aNumOfBalls, NULL);
}*/

void BallDetect::getBallData(cv::Mat& aSorce, BallData aResult[], int& aNumOfBall, cv::Mat& aResultImg)
{
	cv::Mat labelImage(aSorce.size(), CV_16SC1);
	int detectedBallCounter = 0;
	LabelingBS label;
	label.Exec(aSorce.data, (short *)labelImage.data, aSorce.cols, aSorce.rows, true, mAreaPixelThresh);

	for (int i = 0; i < label.GetNumOfResultRegions(); i++)
	{
		RegionInfoBS *regioninfo = label.GetResultRegionInfo(i);
		int x1, x2, y1, y2, w, h;
		regioninfo->GetMin(x1, y1);
		regioninfo->GetMax(x2, y2);
		w = x2 - x1;
		h = y2 - y1;
		float x, y;
		regioninfo->GetCenter(x, y);
		int center_x = (int)x;
		int center_y = (int)y;

		//printf("%d\n", regioninfo->GetNumOfPixels());
		if (labelImage.at<short>(center_y, center_x) == 0)
		{
			continue;
		}

		int ball_width = 0, ball_height = 0;
		/*if (label_img.at<short>(center_y, x2) == )
		{

		}*/


		int step_width = 1;
		/* 中心から 右方向へ探索 */
		int check_x, check_y;
		for (check_x = center_x; check_x < labelImage.cols; check_x += step_width)
		{
			if (int val = labelImage.at<short>(center_y, check_x) == 0)
			{
				break;
			}
		}
		ball_width += check_x - center_x;

		/* 中心から左方向へ探索 */
		for (check_x = center_x; 0 <= check_x; check_x -= step_width)
		{
			if (int val = labelImage.at<short>(center_y, check_x) == 0)
			{
				break;
			}
		}
		ball_width += center_x - check_x;

		/* 中心から下方向へ探索 */
		for (check_y = center_y; check_y < labelImage.rows; check_y += step_width)
		{
			if (labelImage.at<short>(check_y, center_x) == 0)
			{
				break;
			}
		}
		ball_height += check_y - center_y;

		/* 中心から上方向へ探索*/
		for (check_y = center_y; 0 <= check_y; check_y -= step_width)
		{
			if (labelImage.at<short>(check_y, center_x) == 0)
			{
				break;
			}
		}
		ball_height += center_y - check_y;

		if (abs(ball_width - ball_height) < (ball_width + ball_height) >> 3 
			&& abs(w - ball_width) < 10
			&& abs(h - ball_height) < 10)
		{
			//ボールとする
			aResult[detectedBallCounter].x = center_x;
			aResult[detectedBallCounter].y = center_y;
			aResult[detectedBallCounter].diameter = (ball_height+ball_width)/2;
			aResult[detectedBallCounter].numOfPixels = regioninfo->GetNumOfPixels();
			
			// ボールの位置を四角で囲む
			cv::rectangle(aResultImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255));
			/*if (aResultImg != NULL)
			{
				cv::rectangle(aResultImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255));
			}
			else{
				cv::rectangle(aSorce, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255));
			}*/

			// 探すボールはひとつだけ
			detectedBallCounter++;
			if (mLimitOfBall <= detectedBallCounter)
			{
				break;
			}
		}
	}
	aNumOfBall = detectedBallCounter;
	//cv::imshow("result", *mResultImg);
}


void BallDetect::threshold(const cv::Mat& aSorce, cv::Mat& aDest, const ThreshData& thresh)
{
	std::vector<cv::Mat> channel;
	cv::Mat hsvImg, hue1, hue2, hue, saturation1, saturation2, saturation, value1, value2, value, hue_saturation;
	cv::cvtColor(aSorce, hsvImg, CV_BGR2HSV);
	cv::split(hsvImg, channel);
	
	if (thresh.hueUpper > thresh.hueLower)
	{
		cv::threshold(channel[0], hue1, thresh.hueUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[0], hue2, thresh.hueLower, 255, CV_THRESH_BINARY);
		cv::threshold(channel[1], saturation1, thresh.satUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[1], saturation2, thresh.satLower, 255, CV_THRESH_BINARY);
		cv::threshold(channel[2], value1, thresh.valUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[2], value2, thresh.valLower, 255, CV_THRESH_BINARY);
		cv::bitwise_and(hue1, hue2, hue);
		cv::bitwise_and(saturation1, saturation2, saturation);
		cv::bitwise_and(value1, value2, value);
		cv::bitwise_and(hue, saturation, hue_saturation);
		cv::bitwise_and(hue_saturation, value, aDest);
	}
	else
	{
		cv::threshold(channel[0], hue1, thresh.hueUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[0], hue2, thresh.hueLower, 255, CV_THRESH_BINARY);
		cv::threshold(channel[1], saturation1, thresh.satUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[1], saturation2, thresh.satLower, 255, CV_THRESH_BINARY);
		cv::threshold(channel[2], value1, thresh.valUpper, 255, CV_THRESH_BINARY_INV);
		cv::threshold(channel[2], value2, thresh.valLower, 255, CV_THRESH_BINARY);
		cv::bitwise_or(hue1, hue2, hue);
		cv::bitwise_and(saturation1, saturation2, saturation);
		cv::bitwise_and(value1, value2, value);
		cv::bitwise_and(hue, saturation, hue_saturation);
		cv::bitwise_and(hue_saturation, value, aDest);
	}
}

void BallDetect::setAreaPixelThresh(int aValue)
{
	mAreaPixelThresh = aValue;
}
