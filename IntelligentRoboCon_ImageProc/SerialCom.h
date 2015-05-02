#pragma once

#include <thread>
#include <mutex>
#include <windows.h>

class SerialCom
{
public:
	SerialCom();
	SerialCom(int aRxBufferSize);
	~SerialCom();
	int open(int aSerialPortNum, int aBoundRate);
	int close();
	int sendArray(unsigned char *aTxData, int aTxDataSize);
	int sendChar(unsigned char aTxChar);
	int readRxBuffer(unsigned char *aReadBuffer, int aReadBufferSize);
	int clearRxBuffer();
	int test();
private:
	void init(int aRxBufferSize);
	enum EComState{
		CLOSE = 0,
		OPEN,
	};
	EComState mComState;

	/* シリアル通信の設定 */
	int mSerialPortNum;
	int mBoundRate;
	HANDLE mPortHandle;

	/* マルチスレッドで扱うやつ */
	std::thread *mComThread;
	std::mutex mutex;
	bool mThreadExitFlag;
	void threadMain();

	/* 読み込み */
	unsigned char *mRxBuffer;
	int mRxBufLength;
	int mRxBufferSize;

	/* 書き込み 
	unsigned char *mTxBuffer;
	int mTxBufferSize;
	int mTxWriteIndex;
	int mUnWroteDataSize;*/
	
};
