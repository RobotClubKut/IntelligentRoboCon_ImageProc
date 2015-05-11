#include <cstdio>
#include "IntelligentRobo.h"
#include "BallDetect.h"

//#define UNUSE_COM

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

	mMode = APPROACH;
	mNumOfTookBalls = 0;

#ifndef UNUSE_COM
	mSerial = new SerialCom;
	mSerial->open(3, 9600);
	mRxBuffer = new uint8(10);
	//mRxLength = 0;
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


/*void IntelligentRobo::approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData)
{
	if (mNumOfTookBalls >= 15)
	{
		mMode = LINE_TRACE;
		aTxData.LineTraceMode.targetPos = 0;
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
		aTxData.ApproachMode.move = 0;
		aTxData.ApproachMode.turn = (x > 0) ? 0 : 1;
		aTxData.ApproachMode.speed = (abs(x) < FRONT_MARGIN_PX + 10) ? LOW : NORMAL;
		frontCounter = 0;
	}
	else{
		// おそらく正面にボールがある
		frontCounter++;
		if (frontCounter < 5)
		{
			// 旋回しすぎているかもしれないので止まってちょっと待つ
			aTxData.ApproachMode.speed = STOP;
			return;
		}
		// 絶対正面にボールがある
		static const int MIN_YPOS = TAKE_YPOS_PX - TAKE_YPOS_MARGIN_PX;
		static const int MAX_YPOS = TAKE_YPOS_PX + TAKE_YPOS_MARGIN_PX;
		if (MIN_YPOS < y && y < MAX_YPOS)
		{
			// ボールがハンドでとれる位置にある
			aTxData.ApproachMode.speed = STOP;
			mMode = CATCH;
			return;
		}
		// 前進または後退してハンドでとれる距離に移動する
		aTxData.ApproachMode.move = 1;
		aTxData.ApproachMode.run = (y < TAKE_YPOS_PX) ? FORWARD : BACKWARD;
		aTxData.ApproachMode.speed = LOW;
	}
}
*/

void IntelligentRobo::setTxData(uint8 aMode, uint16 aData)
{
	// test
	int sum = 0;
	uint8 data1 = (uint8)(aData & 0x00ff);
	//uint16 dbg = (uint8)(aData & 0xff00);
	uint8 data2 = (uint8)(aData >> 8);
	//sum = 0x55 + aMode + aData;
	mSerial->sendChar(0x55);
	mSerial->sendChar(aMode);
	mSerial->sendChar(data1);
	mSerial->sendChar(data2);
	mSerial->sendChar(sum);
	printf("[0:%4d] [1:%4d] [2:%4d] [3:%4d] [4:%4d]\n", 0x55, aMode, data1, data2, sum);
}

bool IntelligentRobo::getRxData(uint8& aMode, uint16& aData)
{
	const int packetSize = 5;
	static int rxBytes = 0;
	static uint8 tmpRx[32];
	int readBytes;
	const int rxBufferSize = 32;
	uint8 rxBuffer[rxBufferSize];
	uint16 tmpData;

	mRxFlag &= ~ERROR;
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
				mRxFlag |= ERROR; 
			}
			aMode = tmpRx[1];
			tmpData = tmpRx[2] | (tmpRx[3] << 8);
			aData = tmpData;
			printf("[0:%4d] [1:%4d] [2:%4d] [3:%4d] [4:%4d]\n",
				tmpRx[0],
				tmpRx[1],
				tmpRx[2],
				tmpRx[3],
				tmpRx[4]
				);
			return true;
		}
	}
	if (mRxFlag & ERROR)
	{
		printf("%4d error\n", rxBytes);
	}
	return false;
}


void IntelligentRobo::approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData)
{
	
}

void IntelligentRobo::lineTrace(TransmitData& aTxData)
{
	if (aTxData.LineTraceMode.arrivalFlag != 0)
	{
		mMode = SHOOT;
	}
}


void IntelligentRobo::takeTheBall(TransmitData& aTxData)
{
	if (aTxData.CatchMode.stateCatch == SUCCESS)
	{
		std::cout << "ボールとれたお" << std::endl;
		mNumOfTookBalls++;
		aTxData.whole = 0;
		if (mNumOfTookBalls >= 15)
		{
			std::cout << "全部とれたネ" << std::endl;
			mMode = LINE_TRACE;
			// 今はてきとー
			aTxData.LineTraceMode.targetPos = 3;
		}
		mMode = APPROACH;
		aTxData.ApproachMode.speed = STOP;
	}
}


void IntelligentRobo::shoot(TransmitData& aTxData)
{
	static const Color color[3] = { RED, BLUE, YELLOW };
	static const int shootPos[3] = { 3, 2, 1 };
	static int colorIndex = 0;
	aTxData.ShootMode.color = color[colorIndex];
	if (aTxData.ShootMode.shotFlag != 0)
	{
		// ボールを排出できたとき
		aTxData.whole = 0;
		mMode = LINE_TRACE;
		aTxData.LineTraceMode.targetPos = shootPos[colorIndex];
		colorIndex++;
	}
}


int IntelligentRobo::searching()
{
	static unsigned char state = 0;
	int ret = 0;

	// distance of nearest ball
	// front right left
	int distances[3];
	uint8 command = 0;
	uint16 data = 0;

	switch (state)
	{
	case 0:
		// do image proc
		// it's recording distance of nearest ball
		// ...

		// Turn right
		setTxData(mMode, (uint16)SEARCHING_ANGLE);
		state++;
		break;
	case 1:
		if (!(mRxFlag & UNREAD))
		{
			break;
		}
		if (getRxData(command, data))
		{
			// do image proc

			// next action
			setTxData(mMode, (uint16)-90);
			state++;
		}
		break;
	case 2:
		if (!(mRxFlag & UNREAD))
		{
			break;
		}
		if (getRxData(command, data))
		{
			// do image proc
			// Choose the nearest ball from the recorded ones
			int max = 0;
			int index = 0;
			for (int i = 0; i < 3; i++)
			{
				if (max < distances[i])
				{
					max = distances[i];
					index = i;
				}
			}
			switch (index)
			{
			case 0:
				setTxData(mMode, SEARCHING_ANGLE);
				break;
			case 1:
				setTxData(mMode, 2 * SEARCHING_ANGLE);
				break;
			case 2:
				break;
			}
			state++;
		}
		break;
	case 3:
		if (!(mRxFlag & UNREAD))
		{
			break;
		}
		// Check the action has been completed.
		// ....

		break;
	}
	return 0;
}


int IntelligentRobo::intelligence(const BallData aBalls[], int aNumOfBalls)
{
	static uint16 counter = 255;
	uint8 command, rxCommand;
	uint16 data, rxData;
	if (!(mRxFlag & WAITING))
	{
		setTxData(0x05, counter++);
		printf("sending\n");
		mRxFlag |= WAITING;
	}
	if (getRxData(rxCommand, rxData))
	{
		printf("receiving\n");
		mRxFlag &= ~WAITING;
		command = rxCommand;
		data = rxData;
	}
	return 0;

	mMode = APPROACH;
	int s;

	switch (mMode)
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
	}

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
	//mSerial->sendChar(txData.whole);
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