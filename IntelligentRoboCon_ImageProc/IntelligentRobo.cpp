#include <cstdio>
#include "IntelligentRobo.h"

#define UNUSE_COM

IntelligentRobo::IntelligentRobo()
{
#ifdef CAMERA_WIDTH_PX
	mCameraWidth = CAMERA_WIDTH_PX;
#else
	printf("\"CAMERA_WIDTH_PX\"が指定されていません.\n");
	mCameraWidth = 320;
#endif
#ifdef CAMERA_HEIGHT_PX
	mCameraHeight = CAMERA_HEIGHT_PX;
#else
	printf("\"CAMERA_HEIGHT_PX\"が指定されていません.\n");
	mCameraHeight = 240;
#endif
	mCameraCenterX = mCameraWidth / 2;
	mCameraCenterY = mCameraHeight / 2;

	mSequence = SEQ_WAIT;
	mNumOfTookBalls = 0;

#ifndef UNUSE_COM
	mSerial = new SerialCom;
	mSerial->open(3, 9600);
#endif
}


IntelligentRobo::~IntelligentRobo()
{
#ifndef UNUSE_COM
	delete mSerial;
#endif
}


void IntelligentRobo::cvtCamera2Robot(int& ax, int& ay)
{
	//ax -= mCameraCenterX;
	//ay -= mCameraCenterY;
	ax -= 320;
	ay -= 240;
}


void IntelligentRobo::approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData)
{
	if (mNumOfTookBalls >= 15)
	{
		mSequence = SEQ_TRACE;
		aTxData.Bit.speed = 0;
		return;
	}
	if (aNumOfBalls == 0)
	{
		//ボールが見当たらないので線上に戻って前進
		return;
	}
	// 取るボールを選択(一番近いやつ)
	int nearestBallIndex = 0;
	for (int i = 1; i < aNumOfBalls; i++)
	{
		if (aBalls[nearestBallIndex].y < aBalls[i].y)
		{
			nearestBallIndex = i;
		}
	}
	int x = aBalls[nearestBallIndex].x;
	int y = aBalls[nearestBallIndex].y;
	cvtCamera2Robot(x, y);

	static int frontCounter = 0;	// 正面にいる状態のカウンタ
	if (abs(x) > FRONT_MARGIN_PX)
	{
		// 一番近いボールが正面でない
		aTxData.Bit.moveFlag = 0;
		aTxData.Bit.leftOrRight = (x > 0) ? 0 : 1;
		aTxData.Bit.speed = (abs(x) < FRONT_MARGIN_PX + 10) ? 1 : 2;
		frontCounter = 0;
	}
	else{
		// おそらく正面にボールがある
		frontCounter++;
		if (frontCounter < 5)
		{
			// 旋回しすぎているかもしれないので止まってちょっと待つ
			aTxData.Bit.speed = 0;
			return;
		}
		// 絶対正面にボールがある
		static const int MIN_YPOS = TAKE_YPOS_PX - TAKE_YPOS_MARGIN_PX;
		static const int MAX_YPOS = TAKE_YPOS_PX + TAKE_YPOS_MARGIN_PX;
		if (MIN_YPOS < y && y < MAX_YPOS)
		{
			// ボールがハンドでとれる位置にある
			aTxData.Bit.speed = 0;
			mSequence = SEQ_TAKE;
			return;
		}
		// 前進または後退してハンドでとれる距離に移動する
		aTxData.Bit.moveFlag = 1;
		aTxData.Bit.fowardOrBack = (y < TAKE_YPOS_PX) ? 1 : 0;
		aTxData.Bit.speed = 1;
	}
}


void IntelligentRobo::lineTrace(TransmitData& aTxData)
{

}


void IntelligentRobo::takeTheBall(TransmitData& aTxData)
{

}


void IntelligentRobo::intelligence(const BallData aBalls[], int aNumOfBalls)
{
	TransmitData txData, rxData;
	txData.whole = 0;
	rxData.whole = 0;
	mSequence = SEQ_APPROACH;

	switch (mSequence)
	{
	case SEQ_WAIT:
		break;
	case SEQ_TRACE:
		lineTrace(txData);
		break;
	case SEQ_APPROACH:
		approachTheBall(aBalls, aNumOfBalls, txData);
		break;
	case SEQ_TAKE:
		takeTheBall(txData);
		break;
	default:
		std::cerr << "意味わからん" << std::endl;
	}

	printf("mode:%d  moveFlag:%d  ForB:%d  LorR:%d  speed:%d\n",
		txData.Bit.mode, 
		txData.Bit.moveFlag, 
		txData.Bit.fowardOrBack, 
		txData.Bit.leftOrRight, 
		txData.Bit.speed
		);

#if 0
	if (aNumOfBalls <= 0)
	{
		return;
	}

	if (mSequence == SEQ_WAIT)
	{
		/* memo ここらへんで データ受信 */
		/* memo 受信できなかった場合も 記述しないといかん */

		/* 制御が返ってくるまで待つ */
		if (rxData.Bit.mode != EMODE_PC_CONTROLL)
		{
			return;
		}
	}
	else
	if (mSequence == SEQ_SEARCH)
	{
		if (aNumOfBalls != 0)
		{
			mSequence = SEQ_APPROACH;
			txData.Bit.speed = 0;
		}
		else{

		}
	}
	else
	if (mSequence == SEQ_APPROACH)
	{
		const int dTrelance = 5;
		if (aNumOfBalls == 0)
		{
			mSequence = SEQ_SEARCH;
		}
		int x = aBalls[0].x;
		int y = aBalls[0].y;
		int d = aBalls[0].diameter;
		cvtCamera2Robot(x, y);
		//printf("%d %d\n", x, d);
		if (abs(x) <= 4)
		{
			txData.Bit.moveFlag = 1;
			txData.Bit.fowardOrBack = (d >= 55) ? 1 : 0;
			txData.Bit.speed = 1;
			if (abs(d - 55) <= dTrelance)
			{
				txData.Bit.speed = 0;
			}
		}
		else if (4 < abs(x))
		{
			txData.Bit.moveFlag = 0;
			txData.Bit.leftOrRight = (x > 0) ? 0 : 1;
			txData.Bit.speed = (abs(x) < 8) ? 1 : 2;
		}
		/*printf("%d %d %d %d %4d %4d %4d %4d\n",
			txData.Bit.moveFlag,
			txData.Bit.fowardOrBack,
			txData.Bit.leftOrRight,
			txData.Bit.speed,
			mCameraCenterX,
			mCameraCenterY,
			x,
			y);*/
	}
#endif

	/* ここらへんでtxDataを送信する */
#ifndef UNUSE_COM
	mSerial->sendChar(txData.whole);
#endif
}

void IntelligentRobo::drawMark(cv::Mat& aImg)
{
	int centerX = aImg.cols >> 1;
	int centerY = aImg.rows >> 1;
	cv::line(aImg, cv::Point(centerX, 0), cv::Point(centerX, aImg.rows), cv::Scalar(0, 0, 255));
	cv::line(aImg, cv::Point(0, centerY + TAKE_YPOS_PX), cv::Point(aImg.cols, centerY + TAKE_YPOS_PX), cv::Scalar(0, 0, 255));
}