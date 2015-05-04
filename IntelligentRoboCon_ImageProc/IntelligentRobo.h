#pragma once
#include "BallDetect.h"
#include "SerialCom.h"			/* 通信 */

/* カメラ */
#define CAMERA_WIDTH_PX 640
#define CAMERA_HEIGHT_PX 480

/* ボールが正面にあるか否かを判断する範囲 マージン */
#define FRONT_MARGIN_PX 18

/* 取るボールのy座標とマージン */
#define TAKE_YPOS_PX 150
#define TAKE_YPOS_MARGIN_PX 10


/* 全体をこのPCが制御するか、PSoCのマスターが制御するかの状態 */
enum Mode
{
	EMODE_PC_CONTROLL = 0,
	EMODE_MASTER_CONTROLL,
};

union TransmitData
{
	unsigned char whole;
	struct {
		Mode mode : 2;			// Modeが増えるかもしれないので2bitとっておく
		unsigned char pad : 1;	// 未使用のビット
		unsigned char moveFlag : 1;		// 旋回のとき0, 移動のとき1

		unsigned char fowardOrBack : 1;	// 前進のとき1, 後退のとき0
		unsigned char leftOrRight : 1;	// 左旋回のとき1, 右旋回のとき0
		unsigned char speed : 2;		// 停止のとき0, 1~3で速度調整
	}Bit;
};


class IntelligentRobo
{
public:
	IntelligentRobo();
	~IntelligentRobo();
	void intelligence(const BallData aBalls[], int aNumOfBall);
	void drawMark(cv::Mat& aImg);
private:
	SerialCom *mSerial;
	int mCameraWidth;
	int mCameraHeight;
	int mCameraCenterX;
	int mCameraCenterY;

	enum ESeq
	{
		SEQ_WAIT = 0,
		SEQ_TRACE,
		SEQ_APPROACH,
		SEQ_SEARCH,
		SEQ_TAKE,
		SEQ_SHOOT,
	};
	ESeq mSequence;

	int mNumOfTookBalls;
	void cvtCamera2Robot(int& ax, int& ay);
	void approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData);
	void lineTrace(TransmitData& aTxData);
	void takeTheBall(TransmitData& aTxData);
};

