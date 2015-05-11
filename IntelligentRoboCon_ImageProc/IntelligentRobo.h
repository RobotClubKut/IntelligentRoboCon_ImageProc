#pragma once
#include "BallDetect.h"
#include "SerialCom.h"			/* 通信 */

/* カメラ */
#define CAMERA_WIDTH_PX 320
#define CAMERA_HEIGHT_PX 240

/* ボールが正面にあるか否かを判断する範囲 マージン */
#define FRONT_MARGIN_PX 18

/* 取るボールのy座標とマージン */
#define TAKE_YPOS_PX 50
#define TAKE_YPOS_MARGIN_PX 10

/* ボールを探す時の旋回する角度 */
#define SEARCHING_ANGLE 45

/* RxFlag */
#define UNREAD  0x01
#define WAITING 0x02
#define ERROR   0x04


typedef unsigned char uint8;
typedef unsigned short uint16;

/* 命令の出し方 */
enum Mode
{
	INITIAL = 0,
	SHOOTING_TENNIS_BALL,
	LINE_TRACE,
	SEARCHING,
	APPROACH,
	CATCH,
	SHOOT,
};

/*
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
};*/

typedef enum
{
	STOP = 0,
	LOW,
	NORMAL,
	HIGH,
}Speed;

typedef enum
{
	RED = 0,
	YELLOW,
	BLUE,
}Color;

typedef enum
{
	REGULAR = 0,
	FAILED,
	SUCCESS,
}StateCatch;

enum Run
{
	BACKWARD = 0,
	FORWARD,
};

typedef union
{
	unsigned char whole;
	struct
	{
		Mode mode : 2;
		uint8 bit5 : 1;
		uint8 bit4 : 1;
		uint8 bit3 : 1;
		uint8 bit2 : 1;
		uint8 bit1 : 1;
		uint8 bit0 : 1;
	}Bit;

	struct
	{
		Mode modeSelect : 2;
		uint8 pad : 1;
		uint8 move : 1;
		uint8 run : 1;
		uint8 turn : 1;
		Speed speed : 2;
	}ApproachMode;

	struct
	{
		Mode modeSelect   : 2;
		uint8 pad         : 2;
		uint8 arrivalFlag : 1;
		uint8 targetPos   : 3;
	}LineTraceMode;

	struct
	{
		Mode modeSelect : 2;
		Color color : 2;
		StateCatch stateCatch : 2;
		uint8 pad : 2;
	}CatchMode;

	struct
	{
		Mode modeSelect : 2;
		uint8 pad : 3;
		uint8 shotFlag : 1;
		uint8 color : 2;
	}ShootMode;
}TransmitData;


class IntelligentRobo
{
public:
	IntelligentRobo();
	~IntelligentRobo();
	int intelligence(const BallData aBalls[], int aNumOfBall);
	void drawMark(cv::Mat& aImg);
private:
	SerialCom *mSerial;
	int mCameraWidth;
	int mCameraHeight;
	int mCameraCenterX;
	int mCameraCenterY;

	unsigned char mRxFlag;
	unsigned char mTxFlag;

	Mode mMode;

	int mNumOfTookBalls;

	void cvtCamera2Robot(int& ax, int& ay);
	void approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData);
	void lineTrace(TransmitData& aTxData);
	void takeTheBall(TransmitData& aTxData);
	void shoot(TransmitData& aTxData);

	//new
	int searching();
	void setTxData(uint8 aMode, uint16 aData);
	bool getRxData(uint8& aMode, uint16& aData);
	//void filteringOutside(const BallDetect& aYellow, cv::Mat& aDst);

	// 受信後に一時的にいれておくバッファ的な
	uint8 *mRxBuffer;
	//uint8 mRxLength;

	// 送信待ちの一時的なバッファ的な
	//uint8 mTxCommand;
	//uint16 mTxData;
};

