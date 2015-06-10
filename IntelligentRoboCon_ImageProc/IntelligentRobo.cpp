#include <cstdio>
#include "IntelligentRobo.h"
#include "BallDetect.h"

#define COMMAND_MODE 0x7fu
#define COMMAND_FLAG 0x80u

//#define UNUSE_COM


IntelligentRobo::IntelligentRobo(ThreshData& aRed, ThreshData& aBlue, ThreshData& aYellow)
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
#ifndef UNUSE_COM
	mSerial = new SerialCom;
	mSerial->open("COM3", 115200);
	mRxBuffer = new uint8(10);
	//mRxLength = 0;
#endif

	mCameraCenterX = mCameraWidth / 2;
	mCameraCenterY = mCameraHeight / 2;

	mMode = INIT;
#ifndef DEBUG
	mState = 0;
#else
	mState = 1;
#endif
	mNumOfTookBalls = 0;

	ballRed = new BallData[10];
	ballBlue = new BallData[10];
	ballYellow = new BallData[10];

	mThreshRed = aRed;
	mThreshBlue = aBlue;
	mThreshYellow = aYellow;

	mRed = 0;
	mBlue = 0;
	mYellow = 0;

	mSeekState = 1;
	mNonBallFlag = false;
	mColor = -1;

	detect = new BallDetect(10, 200);
}


IntelligentRobo::~IntelligentRobo()
{
#ifndef UNUSE_COM
	delete mSerial;
#endif
	delete detect;
	//delete mRxBuffer;
}


void IntelligentRobo::cvtCamera2Robot(int& ax, int& ay)
{
	ax -= mCameraCenterX;
	ay -= mCameraCenterY;
	//ax -= 320;
	//ay -= 240;
}


int IntelligentRobo::openCon()
{
#ifndef UNUSE_COM
	uint16 txData = 0x0000;
	uint8 command, rxCommand;
	uint16 data, rxData;

	if (!(mRxFlag & WAITING))
	{
		setTxData(0xaa, 0x1111);
		printf("test...\n");
	}

	while (true)
	{
		if (!getRxData(rxCommand, rxData))
		{
			continue;
		}

		if (rxCommand == 0xAA && rxData == 0xFFFF)
		{
			printf("ok\n");
			break;
		}
	}

	setTxData(0x00, 0x0000);
	mMode = INIT;
#endif /* UNUSE COM */
	return 0;
}

void IntelligentRobo::setTxData(uint8 aMode, uint16 aData)
{
	// test
	int sum = 0;
	uint8 buf[255];
	uint8 data1 = (uint8)(aData & 0x00ff);
	uint8 data2 = (uint8)(aData >> 8);
	//sum = 0x55 + aMode + aData;
	mRxFlag |= WAITING;

	buf[0] = 0x55;
	buf[1] = aMode;
	buf[2] = (uint8)(aData & 0x00ff);
	buf[3] = (uint8)(aData >> 8);
	buf[4] = 0;
	
	mSerial->sendArray(buf, 5);
	
	/*mSerial->sendChar(0x55);
	mSerial->sendChar(aMode);
	mSerial->sendChar(data1);
	mSerial->sendChar(data2);
	mSerial->sendChar(sum);*/
#ifdef DEBUG_TRANSMITTED_DATA
	printf("TRANSMITTED: [0:%4x] [1:%4x] [2:%4x] [3:%4x] [4:%4x]\n", 0x55, aMode, buf[2], buf[3], buf[4]);
#endif
}

bool IntelligentRobo::getRxData(uint8& aMode, uint16& aData)
{
	return mSerial->getLastRxData(aMode, aData);
}

#if 0
bool IntelligentRobo::getRxData(uint8& aMode, uint16& aData)
{
	const int packetSize = 5;
	static int rxBytes = 0;
	static uint8 tmpRx[32];
	int readBytes;
	const int rxBufferSize = 32;
	uint8 rxBuffer[rxBufferSize];
	uint16 tmpData;

	mRxFlag &= ~WAITING;
	//mRxFlag &= ~ERROR;
	rxBuffer[0] = 0;
	rxBuffer[1] = 0;
	rxBuffer[2] = 0;
	rxBuffer[3] = 0;
	rxBuffer[4] = 0;
	readBytes = mSerial->readRxBuffer(rxBuffer, 32);
	
	for (int i = 0; i < readBytes; i++)
	{
		switch (rxBytes)
		{
		case 0:
			if (rxBuffer[i] == 0x55)
			{
				tmpRx[rxBytes++] = rxBuffer[i];
			}
			break;
		case 1:
			tmpRx[rxBytes++] = rxBuffer[i];
			break;
		case 2:
			tmpRx[rxBytes++] = rxBuffer[i];
			break;
		case 3:
			tmpRx[rxBytes++] = rxBuffer[i];
			break;
		case 4:
			tmpRx[rxBytes] = rxBuffer[i];
			rxBytes = 0;
			if (i + 1 != readBytes)
			{
				//error
				//mRxFlag |= ERROR;
				printf("%4d error\n", readBytes);
			}
			aMode = tmpRx[1];
			tmpData = tmpRx[2] | (tmpRx[3] << 8);
			aData = tmpData;
#ifdef DEBUG_RECEIVED_DATA
			printf("RECEIVED:    [0:%4x] [1:%4x] [2:%4x] [3:%4x] [4:%4x]\n",
				tmpRx[0],
				tmpRx[1],
				tmpRx[2],
				tmpRx[3],
				tmpRx[4]
				);
#endif /* DEBUG_RECEIVED_DATA */
			return true;
		}
	}
	//printf("%4d\n", rxBytes);
	/*if (mRxFlag & ERROR)
	{
		printf("%4d error\n", rxBytes);
	}*/
	return false;
}
#endif

double IntelligentRobo::chooseBall(BallData *balls, int& numOfBalls, int& x, int& y)
{
	if (numOfBalls == 0)
	{
		return 0.0;
	}
	/* 一番近いボールを決定する */
	int nearest = 0;
	double nearestDist = 9999;
	for (int i = 0; i < numOfBalls; i++)
	{
		int tx = balls[i].x;
		int ty = balls[i].y;
		tx -= mCameraCenterX;
		ty -= CAMERA_HEIGHT_PX;
		double dist = sqrt(tx*tx + ty*ty);
		if (dist < nearestDist)
		{
			nearestDist = dist;
			nearest = i;
			x = balls[i].x;
			y = balls[i].y;
			cvtCamera2Robot(x, y);
		}
	}
#ifdef DEBUG_APPROACH
	cout << "dist= " << nearestDist << endl;
#endif
	if (nearestDist >= APPROACH_DIST)
	{
		numOfBalls = 0;
	}

	return nearestDist;
}

//void IntelligentRobo::approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData)
NEXT_MODE IntelligentRobo::approachTheBall(Mat& aSrc, uint16& aTxData)
{
	Mat aBinImg;
	BallData balls[32];
	static int stage = 0;
	static int invalidCounter = 0;
	int numOfBalls = 0, numOfBallsY, numOfBallsB, numOfBallsR;
	double nearestDist = 999.9, dist;

	int x, y, xx, yy;
	switch (mColor)
	{
	case BALL_YELLOW:
		detect->threshold(aSrc, aBinImg, mThreshYellow);
		detect->getBallData(aBinImg, balls, numOfBalls, aSrc);
		chooseBall(balls, numOfBalls, x, y);
		break;
	case BALL_RED:
		detect->threshold(aSrc, aBinImg, mThreshRed);
		detect->getBallData(aBinImg, balls, numOfBalls, aSrc);
		chooseBall(balls, numOfBalls, x, y);
		break;
	case BALL_BLUE:
		detect->threshold(aSrc, aBinImg, mThreshBlue);
		detect->getBallData(aBinImg, balls, numOfBalls, aSrc);
		chooseBall(balls, numOfBalls, x, y);
		break;
	case -1:
		detect->threshold(aSrc, aBinImg, mThreshYellow);
		detect->getBallData(aBinImg, balls, numOfBallsY, aSrc);
		dist = chooseBall(balls, numOfBallsY, xx, yy);
		if (numOfBallsY != 0 && nearestDist > dist)
		{
			nearestDist = dist;
			mColor = BALL_YELLOW;
			numOfBalls = numOfBallsY;
			x = xx;
			y = yy;
		}
		detect->threshold(aSrc, aBinImg, mThreshBlue);
		detect->getBallData(aBinImg, balls, numOfBallsB, aSrc);
		dist = chooseBall(balls, numOfBallsB, xx, yy);
		if (numOfBallsB != 0 && nearestDist > dist)
		{
			nearestDist = dist;
			mColor = BALL_BLUE;
			numOfBalls = numOfBallsB;
			x = xx;
			y = yy;
		}
		detect->threshold(aSrc, aBinImg, mThreshRed);
		detect->getBallData(aBinImg, balls, numOfBallsR, aSrc);
		dist = chooseBall(balls, numOfBallsR, xx, yy);
		if (numOfBallsR != 0 && nearestDist > dist)
		{
			nearestDist = dist;
			mColor = BALL_RED;
			numOfBalls = numOfBallsR;
			x = xx;
			y = yy;
		}
		break;
	}

	if (numOfBalls == 0)
	{
		mColor = -1;
	}

	/* 表示 */
	drawMark(aSrc);
	imshow("result", aSrc);
	imshow("red", aBinImg);

	/* ボールが見つからないとき */
	if (numOfBalls == 0)
	{
		invalidCounter++;
		
		if (invalidCounter >= 5)
		{
			mNonBallFlag = true;
			invalidCounter = 0;
			stage = 0;
			return SEEK;
		}
		return APPROACH;
	}
	invalidCounter = 0;
	mNonBallFlag = false;

	//chooseBall(balls, numOfBalls, x, y);
	
	static int counter = 0;
	
	/* ボールを正面に持ってくる */
	if (stage == 0 || stage == 2)
	{
		if (abs(x) < FRONT_MARGIN_PX)
		{
			counter++;
			if (counter >= 5)
			{
				counter = 0;
				stage++;
			}
			aTxData = MV_STOP;
		}
		else if (x > 0)
		{
			aTxData = MV_RIGHT;
			counter = 0;
		}
		else/* (x < 0) */
		{
			aTxData = MV_LEFT;
			counter = 0;
		}
	}
	/* 一定の距離まで近づくお */
	else if (stage == 1 || stage == 3)
	{
		if (TAKE_YPOS_PX - TAKE_YPOS_MARGIN_PX < y && y < TAKE_YPOS_PX + TAKE_YPOS_MARGIN_PX)
		{
			/*if (stage == 1)
			{
				stage = 0;
				return APPROACH;
			}*/
			counter++;
			if (counter >= 5)
			{
				stage++;
				if (stage != 4)
				{
					stage = 2;
					aTxData = MV_STOP;
					return APPROACH;
				}
				stage = 0;
				counter = 0;
				aTxData = MV_STOP;
				return CATCH;
			}
		}
		else if (y < TAKE_YPOS_PX)
		{
			/* 距離が遠い */
			counter = 0;
			aTxData = MV_FORWARD;
		}
		else /* y > TAKE_YPOS_PX */
		{
			/* 距離が近い */
			counter = 0;
			aTxData = MV_BACKWARD;
		}
	}
#ifdef DEBUG_APPROACH
	//printf("x=%4d, y=%4d, dist=%lf\n", x, y);
	cout << stage << ", " << mColor <<  endl;
#endif
	return APPROACH;
}

NEXT_MODE IntelligentRobo::lineTrace(Mat& aSrc, uint16& aRxData, uint16& aTxData)
{
	NEXT_MODE next = LINE_TRACE;
	/*if (aTxData.LineTraceMode.arrivalFlag != 0)
	{
		mMode = SHOOT;
	}*/
	int line1, line2, line3, line4, diff, ave;
	const int numOfLines = 6;
	int lines[numOfLines];
	Mat binImg;
	Mat src = aSrc;
	unsigned char order1 = 0, order2 = 0;
	int val = 0;
	static int state = 0;
	int ret = 0;
	static int counter = 0;
	const static int limit = 5;

	threshBin(src, binImg, LINE_BLACK);
	senseLine(src, binImg, line1, line2);
	//senseCurveLine(aSrc, binImg);
	
	/* 通常のライントレース ラインからのずれを修正してくかんじ */
	if (state == 0)
	{
		if (mState == 4)
		{
			order1 = order2 = 10;
			if (senseLine3(aSrc, binImg, line3, line4))
			{
				return SHOOT;
			}
			state = 0;
		}
		else{
			state = senseCurveLine(aSrc, binImg);
		}
		if (state == 0)
		{
			int lineDist = LINE_CHECK_DIST_1 - LINE_CHECK_DIST_2;
			diff = line1 - line2;
			int inclination = diff != 0 ? (int)(((double)diff / (double)lineDist) * 90) : 0;

			val = line1 - inclination;
			order1 = val + 90;
			order2 = 0;
			/*
			// line1 line2: -100 ~ +100
			diff = line1 - line2;
			ave = (line1 + line2) / 2;

			if (line1 == INT_MAX || line2 == INT_MAX)
			{
				order1 = line1 > 0 ? 180 : 0;
				order2 = 0;
			}

			val = ave;
			val -= diff << 1;
			val = val > 90 ? 90 : -90 > val ? -90 : val;
			val += 90;

			// val = 0 - 180
			order1 = val;
			order2 = 0;
			*/
		}
	}

	/* 右カーブ検出してからの処理 */
	if (state == 1)
	{
		order1 = 201;
		order2 = 1;
		if ((aRxData & 0xff) == 4)
		{
			state = 3;
		}
	}

	/* 左カーブ検出してからの処理 */
	if (state == 2)
	{
		order1 = 202;
		order2 = 1;
		if ((aRxData & 0xff) == 4)
		{
			state = 3;
		}
	}
	
	/* 90度サイクロイドしてからの修正 */
	if (state == 3)
	{
		order1 = 0;
		if (line2 > 0)
		{
			order2 = 3;
		}
		else{
			order2 = 2;
		}
		
		if (-5 < line2 && line2 < 5)
		{
			if (counter++ >= limit)
			{
				order1 = 90;
				order2 = 0;
				state = 0;
				mState++;
				/*if (mState == 2)
				{
					return SHOOT;
				}else*/
				if (mState == 3)
				{
					//return SEEK;
					next = SEEK;
				}
			}
		}
		else{
			counter = 0;
		}
	}
	aTxData = (order2 << 8) | order1;
	
#ifdef DEBUG_LINE_TRACE
	//printf("%4d, %4d\n", line1, line2);
	//printf("%4d, %4d, %4d\n", ave, diff, val);
	cout << (int)state << " , " << (int)mState << " , " << (int)order2 << endl;
	imshow("red", binImg);
#endif
	//return LINE_TRACE;
	return next;
}


NEXT_MODE IntelligentRobo::takeTheBall(uint16 aRxData, uint16& aTxData)
{
	uint8 rx1 = aRxData & 0x00ff;
	uint8 rx2 = (aRxData & 0xff00) >> 8;
	uint8 tx1 = mColor, tx2 = 0;
	static int stage = 0;
	NEXT_MODE next = CATCH;
	static bool flag = false;

	switch (stage)
	{
	case 0:
		if (rx1 == TAKE_SUCCESS)
		{
			switch (mColor)
			{
			case BALL_YELLOW:
				mYellow++;
				break;
			case BALL_RED:
				mRed++;
				break;
			case BALL_BLUE:
				mBlue++;
				break;
			}
			if (mRed == 5 &&
				mBlue == 5 &&
				mYellow == 5)
			{
				stage++;
			}
			else{
				next = SEEK;
			}
		}
		else if (rx1 == TAKE_FAILED)
		{
			next = SEEK;
		}
		break;
	case 1:
		/* ボール全部とれたとき 正面を向く*/
		if (mSeekState == 1)
		{
			tx2 = 2;
		} 
		else if (mSeekState == 2)
		{
			tx2 = 3;
		}
		else
		{
			stage++;
			tx2 = 4;
		}

		if (rx1 == 201)
		{
			stage++;
			tx2 = 4;
		}
		break;
	case 2:
		/* 後ろを向く 向けたらライントレース */
		tx1 = 270;
		tx2 = 1;
		if (rx1 == 200)
		{
			next = LINE_TRACE;
			stage = 0;
		}
		break;
	}

#ifdef DEBUG_TAKE
	cout << stage << ", " << tx1 << ", " << mColor << endl;
#endif
	aTxData = (tx2 << 8) | tx1;

	return next;
}


NEXT_MODE IntelligentRobo::shoot(Mat& aSrc, uint16 aRxData, uint16& aTxData)
{
	static const Ball orderToDischarge[4] = { TENNIS, RED, BLUE, YELLOW };
	static int state = 2;
	uint8 tx1, tx2;
	uint8 rx1 = aRxData & 0x00ff;
	uint8 rx2 = (aRxData & 0xff00) >> 8;

	int line1, line2;

	Mat binImg;
	
	switch (state)
	{
	case 2:
		tx1 = 2;
		tx2 = 1;
		if (rx1 == 1)
		{
			state++;
		}
		break;
	case 3:
		tx1 = 3;
		threshBin(aSrc, binImg, 127);
		if (senseLine3(aSrc, binImg, line1, line2))
		{
			state++;
			tx2 = MV_BACKWARD;
		}
		else{
			tx2 = 0;
		}
		break;
	case 4:
		tx1 = 4;
		tx2 = 0;
		if (rx1 == 5)
		{
			
		}
		break;
	}
	printf("%d, %d, %d\n", state, tx1, tx2);
	aTxData = (tx2 << 8) | tx1;
	return SHOOT;
}


NEXT_MODE IntelligentRobo::seek(Mat& aSrc, uint16 aRxData, uint16& aTxData)
{
	//static int state = 1;
	static bool apr = false;
	int ret = 0;

	// distance of nearest ball
	// front right left
	int distances[3];
	uint8 command = 0;
	uint16 data = 0;
	uint8 tx1 = 0, tx2 = 0;
	uint8 rx1 = aRxData & 0x00ff;
	uint8 rx2 = (aRxData & 0xff00) >> 8;

	int lineDist, diff, ave;
	double inclination;
	static int counter = 0;
	int limit;

	Mat binImg;

	if (rx2 == 200)
	{
		return LINE_TRACE;
	}

	switch (mSeekState)
	{
	case 1:
		/* 左を見る */
		tx1 = 90 - SEARCHING_ANGLE;
		tx2 = mSeekState;
		if (rx2 == 1 && rx1 == 1 && !mNonBallFlag/*!apr*/)
		{
			/* ここでカメラでボールを探す */
			/* 一定の距離内にあればmodeをapproach */
			//apr = true;
			return APPROACH;
		}
		if (mNonBallFlag)
		{
			mSeekState++;
			//apr = false;
			mNonBallFlag = false;
		}
		break;
	case 2:
		/* 右を見る */
		tx1 = SEARCHING_ANGLE + 90 + SEARCHING_ANGLE;
		tx2 = mSeekState;
		if (rx2 == 2 && rx1 == 1 && !mNonBallFlag/*!apr*/)
		{
			/* ここでカメラでボールを探す */
			/* 一定の距離内にあればmodeをapproach */
			//apr = true;
			return APPROACH;
		}
		if (mNonBallFlag)
		{
			mSeekState++;
			//apr = false;
			mNonBallFlag = false;
		}
		/*else if (aRxData == 2)
		{
			state++;
		}*/
		break;
	case 3:
		/* 正面を見る */
		tx1 = 90 - SEARCHING_ANGLE;
		tx2 = mSeekState;
		if (rx2 == 3 && rx1 == 1 && !mNonBallFlag/*!apr*/)
		{
			/* ここでカメラでボールを探す */
			/* 一定の距離内にあればmodeをapproach */
			//apr = true;
			return APPROACH;
		}
		if (mNonBallFlag)
		{
			mSeekState++;
			//apr = false;
			mNonBallFlag = false;
		}
		break;
	case 4:
		/* 画像処理でラインからのずれを修正 */
		int line1, line2;
		tx2 = mSeekState;
		threshBin(aSrc, binImg, LINE_BLACK);
		senseLine(aSrc, binImg, line1, line2);
		lineDist = LINE_CHECK_DIST_1 - LINE_CHECK_DIST_2;
		diff = line1 - line2;
		inclination = diff != 0 ? ((double)diff / (double)lineDist) : 0;
		ave = (line1 + line2) / 2;
		limit = 5;

		if(fabs(inclination) < 0.1)
		{
			if(abs(ave) < 5)
			{
				tx1 = MV_STOP;
				counter++;
				if (counter >= limit)
				{
					counter = 0;
					mSeekState++;
				}
			}
			else{
				tx1 = ave > 0 ? MV_RIGHT : MV_LEFT;
			}
		}
		else{
			tx1 = inclination > 0.0 ? MV_ROLLLEFT : MV_ROLLRIGHT;
		}
#if 0
		if (-5 < line1 && line1 < 5)
		{
			if (-5 < line2 && line2 < 5)
			{
				tx1 = MV_STOP;
				/* ボール全部とれたとき */
				/*if (mRed == 5 &&
					mBlue == 5 &&
					mYellow == 5)
				{
					state = 7;
				}
				else{*/
					mSeekState++;
				//}
			}
			else if (line2 > 0)
			{
				tx1 = MV_ROLLLEFT;
			}
			else if (line2 < 0)
			{
				tx1 = MV_ROLLRIGHT;
			}
		}
		else if (line1 > 0)
		{
			tx1 = MV_RIGHT;
		}
		else if (line1 < 0)
		{
			tx1 = MV_LEFT;
		}
#endif
		break;
	case 5:
		tx1 = MV_FORWARD;
		tx2 = mSeekState;
		// tx1前進!
		if (rx2 == 6)
		{
			tx1 = MV_STOP;
			mSeekState = 1;
		}
		break;
	case 7:
		tx1 = 270;
		tx2 = mSeekState;
		if (rx2 == 200)
		{
			return LINE_TRACE;
		}
		break;
	}
	aTxData = ((uint16)tx2 << 8) | (uint16)tx1;
#ifdef DEBUG_SEEK
	//cout << (int)rx2 << endl;
	printf("state=%2d,  rx=%2d %2d,  tx=%2d %2d\n", state, rx1, rx2, tx1, tx2);
#endif
	return SEEK;
}


//int IntelligentRobo::intelligence(const BallData aBalls[], int aNumOfBalls)
int IntelligentRobo::intelligence(Mat& aSrc)
{
	static uint8 flag = 1;
	static Mode old_mMode = INIT;

	uint8 txCommand = 0;
	uint16 txData = 0;

	static uint8 rxCommand;
	static uint16 rxData;

#ifndef UNUSE_COM
	//while (!getRxData(rxCommand, rxData));
	getRxData(rxCommand, rxData);
	/*if (getRxData(rxCommand, rxData))
	{
		old_rxCommand = rxCommand;
		old_rxData = rxData;
	}
	else
	{
		rxCommand = old_rxCommand;
		rxData = old_rxData;
	}*/
#else
	rxCommand = LINE_TRACE;
	mMode = LINE_TRACE;
#endif
	if ((int)(rxCommand & COMMAND_MODE) == (int)mMode)
	{
		old_mMode = mMode;
		switch (mMode)
		{
		case INIT:
#ifndef DEBUG
			if (mState == 1)
			{
				mMode = APPROACH;
			}
#else
			mMode = LINE_TRACE;
#endif
			break;
		case LINE_TRACE:
			mMode = lineTrace(aSrc, rxData, txData);
#ifdef DEBUG_MODEVAL
			cout << "lineTrace" << endl;
#endif
			break;
		case SEEK:
			txData = 0x22;
			mMode = seek(aSrc, rxData, txData);
#ifdef DEBUG_MODEVAL
			cout << "seek" << endl;
#endif
			break;
		case APPROACH:
			//mMode = approachTheBall(aSrc, txData) == 0 ? mMode : SHOOT;
			mMode = approachTheBall(aSrc, txData);
#ifdef DEBUG_MODEVAL
			cout << "approach" << endl;
#endif
			break;
		case CATCH:
			mMode = takeTheBall(rxData, txData);
#ifdef DEBUG_MODEVAL
			cout << "catch" << endl;
#endif
			break;
		case SHOOT:
			mMode = shoot(aSrc, rxData, txData);
#ifdef DEBUG_MODEVAL
			cout << "shoot" << endl;
#endif
			break;
		case FIN:
			break;
		}
	}
	else
	{
		//cout << ((int)rxCommand & COMMAND_MODE) << ", www" << mMode << endl;
	}
	cout << mState << endl;
	txCommand = COMMAND_MODE & (uint8)mMode;
	if (mMode == old_mMode)
	{
		txCommand &= ~COMMAND_FLAG;
	}
	else{
		txCommand |= COMMAND_FLAG;
	}

	/*switch (mMode)
	{
	case INITIAL:
		mMode = SHOOTING_TENNIS_BALL;
		break;
	case SHOOTING_TENNIS_BALL:
		mMode = LINE_TRACE;
		break;
	case LINE_TRACE:
		//lineTrace(txData);
		break;
	case SEARCHING:
		s = searching();
		switch (s)
		{
		case 1:
			mMode = APPROACH;
			break;
		case 2:
			mMode = LINE_TRACE;
			break;
		default:
			break;
		}
		break;
	case APPROACH:
		//approachTheBall(aBalls, aNumOfBalls, txData);
		break;
	case CATCH:
		//takeTheBall(txData);
		break;
	case SHOOT:
		//shoot(txData);
		break;
	default:
		std::cerr << "意味わからん" << std::endl;
	}*/

	//txData.Bit.mode = mMode;

	/*printf("mode:%d  moveFlag:%d  ForB:%d  LorR:%d  speed:%d\n",
		txData.ApproachMode.modeSelect, 
		txData.ApproachMode.move, 
		txData.ApproachMode.run, 
		txData.ApproachMode.turn, 
		txData.ApproachMode.speed
		);
	*/
	/* ここらへんでtxDataを送信する */
#ifndef UNUSE_COM
	setTxData(txCommand, txData);
#endif
	return mNumOfTookBalls;
}



void IntelligentRobo::drawMark(cv::Mat& aImg)
{
	int centerX = aImg.cols >> 1;
	int centerY = aImg.rows >> 1;
	cv::line(aImg, cv::Point(centerX, 0), cv::Point(centerX, aImg.rows), cv::Scalar(0, 0, 255));
	cv::line(aImg, cv::Point(0, centerY + TAKE_YPOS_PX), cv::Point(aImg.cols, centerY + TAKE_YPOS_PX), cv::Scalar(0, 0, 255));
	//cv::line(aImg, cv::Point(0, aImg.rows - aImg.rows / 4), cv::Point(aImg.cols, aImg.rows - aImg.rows / 4), cv::Scalar(0, 0, 255));
}


/*void IntelligentRobo::filteringOutside(const BallDetect& aYellow, cv::Mat& aDst)
{

}*/