#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include "SerialCom.h"
#include <tchar.h>
#include <windows.h>


SerialCom::SerialCom()
{
	init(255);
}


SerialCom::SerialCom(int aRxBufferSize)
{
	init(aRxBufferSize);
}


void SerialCom::init(int aRxBufferSize)
{
	std::cout << "シリアルポート初期化中..." << std::endl;
	mRxBufferSize = aRxBufferSize;
	mRxBuffer = new unsigned char[mRxBufferSize];
	mComState = CLOSE;
	mThreadExitFlag = false;
}


SerialCom::~SerialCom()
{
	if (mComState == OPEN)
	{
		close();
	}
	printf("serial exit\n");
	delete mRxBuffer;
}


void SerialCom::threadMain()
{
	const int TEMP_BUFFER_SIZE = 1024;
	unsigned char tempBuffer[TEMP_BUFFER_SIZE];
	int readBytes = 0;
	while (ReadFile(mPortHandle, tempBuffer, TEMP_BUFFER_SIZE, (LPDWORD)(&readBytes), NULL))
	{
		mutex.lock();
		/* 終了フラグ確認 */
		if (mThreadExitFlag)
		{
			mutex.unlock();
			return;
		}

		for (int i = 0; i < readBytes; i++)
		{
			mRingBuf.push_front(tempBuffer[i]);
			//cout << mRingBuf.size();
		}
		mutex.unlock();
	}
}


int SerialCom::open(char *aPort, int aBoundRate)
{
	std::cout << "シリアルポートを開いています..." << std::endl;
	if (mComState == OPEN)
	{
		std::cerr << "現在開いているポートを閉じてください." << std::endl;
		return -1;
	}

	//mSerialPortNum = aSerialPortNum;
	mBoundRate = aBoundRate;

	/* COMポートを開く */
	//std::string portName("COM");
	//portName += std::to_string(aSerialPortNum);
	TCHAR portName[255];
/*#ifdef UNICODE
	MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, aPort, strlen(aPort), portName, (sizeof portName) / 2);
#else
	strcpy(portName, aPort);
#endif*/
	//if ((mPortHandle = CreateFile(portName,
	if ((mPortHandle = CreateFile("COM6",
	//if ((mPortHandle = CreateFile(_T("\\\\.\\COM3"),
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
		== INVALID_HANDLE_VALUE)
	{
		std::cerr << "ポートを開くことができません." << std::endl;
		DWORD errorCode = GetLastError();
		std::cerr << errorCode << std::endl;
		return -1;
	}

	/* ボーレートの設定 */
	DCB config;
	GetCommState(mPortHandle, &config);
	config.BaudRate = mBoundRate;
	config.ByteSize = 8;
	config.StopBits = ONESTOPBIT;
	config.fParity = NOPARITY;
	SetCommState(mPortHandle, &config);

	/* 通信のタイムアウト設定 */
	COMMTIMEOUTS cto;
	cto.ReadIntervalTimeout = 100;
	cto.ReadTotalTimeoutMultiplier = 0;
	cto.ReadTotalTimeoutConstant = 50;
	cto.WriteTotalTimeoutMultiplier = 0;
	cto.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(mPortHandle, &cto);

	/* 各種データ初期化 */
	mRxBufLength = 0;
	//mTxWriteIndex = 0;
	//mUnWroteDataSize = 0;
	//mRxBuffer = new unsigned char[mRxBufferSize];
	//mTxBuffer = new unsigned char[mTxBufferSize];
	//test();

	/* 別スレッド処理開始 */
	mComState = OPEN;
	mComThread = new std::thread(&SerialCom::threadMain, this);
	
	std::cout << "通信しています..." << std::endl;
	return 0;
}


int SerialCom::close()
{
	std::cout << "シリアルポートを閉じています..." << std::endl;
	if (mComState == CLOSE)
	{
		std::cerr << "\nポートは閉じられています." << std::endl;
		return -1;
	}

	/* 別スレッド処理終了 */
	mThreadExitFlag = true;
	mComState = CLOSE;
	mComThread->join();

	/* ファイルハンドルを閉じる */
	if (mPortHandle != NULL)
	{
		CloseHandle(mPortHandle);
	}

	return 0;
}


int SerialCom::sendArray(unsigned char *aTxData, int aTxDataSize)
{
	int wroteBytes;
	int count = 0;
	while (count < aTxDataSize)
	{
		//cout << "a";
		WriteFile(mPortHandle, aTxData + count, aTxDataSize - count, (LPDWORD)(&wroteBytes), NULL);
		/*int e = GetLastError();
		cout << e;
		cout << "b";
		cout << wroteBytes << endl;*/
		count += wroteBytes;
	}
	return 0;
}


int SerialCom::sendChar(unsigned char aTxChar)
{
	//mutex.lock();
	int wroteByte = 0;
	while (wroteByte == 0)
	{
		WriteFile(mPortHandle, &aTxChar, 1, (LPDWORD)(&wroteByte), NULL);
	}
	//mutex.unlock();
	return 0;
}


int SerialCom::readRxBuffer(unsigned char *aReadBuffer, int aReadBufferSize)
{
	int toReadDataSize;
	int readSize;
	mutex.lock();

	/*if (mRxBufLength == 0)
	{
		mutex.unlock();
		return 0;
	}*/
	//memcpy_s(aReadBuffer, aReadBufferSize, mRxBuffer, mRxBufLength);
	//int length = mRxBufLength;
	int ret = mRingBuf.size();
	//cout << ret << endl;
	int length = ret;
	for (int i = 0; i < aReadBufferSize; i++)
	{
		if (length == 0) break;
		aReadBuffer[i] = mRingBuf[--length];
		mRingBuf.pop_back();
	}
	//mRxBufLength = 0;
	mutex.unlock();
	return ret;
}


bool SerialCom::getLastRxData(unsigned char& aCommand, unsigned short& aData)
{
	bool ret = false;
	int len = mRingBuf.size();
	int i;
	for (i = 4; i < len; i++)
	{
		if (mRingBuf[i] == 0x55)
		{
			aCommand = mRingBuf[i-1];
			aData = (unsigned short)mRingBuf[i - 3] << 8 | (unsigned short)mRingBuf[i - 2];
			printf("RECEIVED:    [0:%4x] [1:%4x] [2:%4x] [3:%4x] [4:%4x]\n",
				0x55,
				mRingBuf[i-1],
				mRingBuf[i-2],
				mRingBuf[i-3],
				mRingBuf[i-4]
				);
			ret = true;
			break;
		}
	}
	if (!ret)
	{
		return false;
	}
	//printf("%d\n", len);
	for (int j = 0; j < len - (i - 4); j++)
	{
		//printf("a");
		mRingBuf.pop_back();
	}
	
	return true;
}


int SerialCom::clearRxBuffer()
{
	mRingBuf.clear();
	return 0;
}


int SerialCom::test()
{
	//std::cout << mTxBuffer[0] << std::endl;
	//std::cout << mTxBuffer[1] << std::endl;

	/*
	mRxHead = 30;
	mUnReadDataSize = 5;
	for (int i = 0; i < mRxBufferSize; i++)
	{
		mRxBuffer[i] = 'A' + i;
	}
	std::cout << mRxBuffer << std::endl;
	*/
	return 0;
}
