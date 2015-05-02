#pragma once
#include "BallDetect.h"
#include "SerialCom.h"			/* �ʐM */

/* �J���� */
#define CAMERA_WIDTH_PX 320
#define CAMERA_HEIGHT_PX 240


/* �S�̂�����PC�����䂷�邩�APSoC�̃}�X�^�[�����䂷�邩�̏�� */
enum Mode
{
	EMODE_PC_CONTROLL = 0,
	EMODE_MASTER_CONTROLL,
};

union TransmitData
{
	unsigned char whole;
	struct {
		Mode mode : 2;			// Mode�������邩������Ȃ��̂�2bit�Ƃ��Ă���
		unsigned char pad : 1;	// ���g�p�̃r�b�g
		unsigned char moveFlag : 1;		// ����̂Ƃ�0, �ړ��̂Ƃ�1

		unsigned char fowardOrBack : 1;	// �O�i�̂Ƃ�1, ��ނ̂Ƃ�0
		unsigned char leftOrRight : 1;	// ������̂Ƃ�1, �E����̂Ƃ�0
		unsigned char speed : 2;		// ��~�̂Ƃ�0, 1~3�ő��x����
	}Bit;
};


class IntelligentRobo
{
public:
	IntelligentRobo();
	~IntelligentRobo();
	void intelligence(const BallData aBalls[], int aNumOfBall);
private:
	SerialCom *mSerial;
	int mCameraWidth;
	int mCameraHeight;
	int mCameraCenterX;
	int mCameraCenterY;

	enum ESeq
	{
		SEQ_WAIT = 0,
		SEQ_STOP,
		SEQ_RUN,
		SEQ_SEARCH,
		SEQ_APPROACH,
		SEQ_TAKE,
	};
	ESeq mSequence;
	void cvtCamera2Robot(int& ax, int& ay);
};

