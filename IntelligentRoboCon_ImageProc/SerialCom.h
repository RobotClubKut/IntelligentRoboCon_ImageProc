#pragma once

#include <thread>
#include <mutex>
#include <deque>

using namespace std;


class SerialCom
{
public:
	SerialCom();
	SerialCom(int aRxBufferSize);
	~SerialCom();
	int open(char *aSerialPortNum, int aBoundRate);
	int close();
	int sendArray(unsigned char *aTxData, int aTxDataSize);
	int sendChar(unsigned char aTxChar);
	int readRxBuffer(unsigned char *aReadBuffer, int aReadBufferSize);
	bool getLastRxData(unsigned char& aCommand, unsigned short& aData);
	int clearRxBuffer();
	int test();
private:
	void init(int aRxBufferSize);
	enum EComState{
		CLOSE = 0,
		OPEN,
	};
	EComState mComState;

	/* �V���A���ʐM�̐ݒ� */
	int mSerialPortNum;
	int mBoundRate;
	HANDLE mPortHandle;

	/* �}���`�X���b�h�ň������ */
	std::thread *mComThread;
	std::mutex mutex;
	bool mThreadExitFlag;
	void threadMain();

	/* �ǂݍ��� */
	unsigned char *mRxBuffer;
	int mRxBufLength;
	int mRxBufferSize;
	deque<unsigned char> mRingBuf;
};
