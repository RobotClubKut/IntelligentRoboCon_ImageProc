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
	ax -= mCameraCenterX;
	ay -= mCameraCenterY;
}



void IntelligentRobo::intelligence(const BallData aBalls[], int aNumOfBalls)
{
	TransmitData txData, rxData;
	txData.whole = 0;
	rxData.whole = 0;
	mSequence = SEQ_APPROACH;

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

	/* ここらへんでtxDataを送信する */
#ifndef UNUSE_COM
	mSerial->sendChar(txData.whole);
#endif
}