#pragma once
#include "BallDetect.h"
#include "SerialCom.h"			/* 通信 */
#include "MyTypedef.h"

/* 通信しない*/
//#define UNUSE_COM

/* デバッグオプション的な何か */
#define DEBUG
//#define DEBUG_TRANSMITTED_DATA
//#define DEBUG_RECEIVED_DATA
//#define DEBUG_LINE_TRACE
//#define DEBUG_APPROACH
//#define DEBUG_SEEK
//#define DEBUG_SHOOT
//#define DEBUG_MODEVAL
#define DEBUG_TAKE

/* カメラ */
#define CAMERA_WIDTH_PX 320
#define CAMERA_HEIGHT_PX 240

/* ボールが正面にあるか否かを判断する範囲 マージン */
#define FRONT_MARGIN_PX 10

/* 取るボールのy座標とマージン */
#define TAKE_YPOS_PX 10
#define TAKE_YPOS_MARGIN_PX 10

/* ボールを探す時の旋回する角度 */
#define SEARCHING_ANGLE 45

/* 黒と判断する閾値 */
#define LINE_BLACK 32

/* RxFlag */
#define UNREAD  0x01
#define WAITING 0x02

/* ライントレース */
#define LINE_CHECK_WIDTH  140
//#define LINE_CHECK_WIDTH 180
#define LINE_CHECK_DIST_1 170
#define LINE_CHECK_DIST_2 130

/* ボール拾い */
#define TAKE_SUCCESS		1
#define TAKE_FAILED			2
#define TAKE_YELLOW_BALL	0
#define TAKE_RED_BALL		1
#define TAKE_BLUE_BALL		2

/* ボールの色 */
#define BALL_YELLOW			0
#define BALL_RED			1
#define	BALL_BLUE			2

/* 取りに */
#define APPROACH_DIST 220


#define NEXT_MODE Mode



/* ライントレース用の関数 */
void threshBin(const Mat aSrc, Mat& aDst, int thresh);
void senseLine(Mat& aSrc, Mat& aBinImg, int& line1, int& line2);
void senseLine2(Mat& aSrc, Mat& aBinImg, int *aLine, int aNum, int aSpace);
int senseCurveLine(Mat& aSrc, Mat& aBinImg);
bool senseLine3(Mat& aSrc, Mat& aBinImg, int& line1, int& line2);


/* 命令の出し方 */
enum Mode
{
	INIT = 1,
	SHOOTING_TENNIS_BALL,
	LINE_TRACE,
	SEEK,
	APPROACH,
	CATCH,
	SHOOT,
	MOVE,
	FIN,
};

enum EMoveCommand
{
	MV_STOP = 1,
	MV_RIGHT,
	MV_LEFT,
	MV_FORWARD,
	MV_BACKWARD,
	MV_ROLLRIGHT,
	MV_ROLLLEFT,
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
	TENNIS,
}Ball;

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
		Ball ball : 2;
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
	IntelligentRobo(ThreshData& aRed, ThreshData& aBlue, ThreshData& aYellow);
	~IntelligentRobo();
	int openCon();
	//int intelligence(const BallData aBalls[], int aNumOfBall);
	int intelligence(Mat& aSrc);
	void drawMark(Mat& aImg);
	//void chooseBall(BallData *balls, int& numOfBalls, int& x, int& y);
private:
	SerialCom *mSerial;
	int mCameraWidth;
	int mCameraHeight;
	int mCameraCenterX;
	int mCameraCenterY;

	unsigned char mRxFlag;
	unsigned char mTxFlag;

	int mRed;
	int mBlue;
	int mYellow;

	int mColor;
	bool mNonBallFlag;

	Mode mMode;
	int mState;
	int mSeekState;
	//uint8 mMode;

	int mNumOfTookBalls;

	BallData *ballRed;
	BallData *ballBlue;
	BallData *ballYellow;
	ThreshData mThreshRed;
	ThreshData mThreshBlue;
	ThreshData mThreshYellow;
	BallDetect *detect;

	void cvtCamera2Robot(int& ax, int& ay);
	double chooseBall(BallData *balls, int& numOfBalls, int& x, int& y);
	//void approachTheBall(const BallData aBalls[], int aNumOfBalls, TransmitData& aTxData);
	NEXT_MODE approachTheBall(Mat& aSrc, uint16& txData);
	NEXT_MODE lineTrace(Mat& aSrc, uint16& aRxData, uint16& aTxData);
	NEXT_MODE seek(Mat& aSrc, uint16 aRxData, uint16& aTxData);
	NEXT_MODE takeTheBall(uint16 aRxData, uint16& aTxData);
	NEXT_MODE shoot(Mat& aSrc, uint16 aRxData, uint16& aTxData);

	// 通信
	void setTxData(uint8 aMode, uint16 aData);
	bool getRxData(uint8& aMode, uint16& aData);

	// 受信後に一時的にいれておくバッファ的な
	uint8 *mRxBuffer;
	//uint8 mRxLength;

	// 送信待ちの一時的なバッファ的な
	//uint8 mTxCommand;
	//uint16 mTxData;
};

