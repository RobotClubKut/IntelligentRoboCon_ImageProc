#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include "SerialCom.h"


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

		if (readBytes > 0)
		{
			/*if (mRxBufferSize - mUnReadDataSize < readBytes)
			{
				std::cerr << "受信データをバッファに書き込めません." << std::endl;
			}*/
			if (mRxBufLength + 1 >= mRxBufferSize)
			{
				mRxBufLength = 0;
			}
			memcpy_s(mRxBuffer + mRxBufLength, mRxBufferSize - mRxBufLength, tempBuffer, readBytes);
			mRxBufLength += readBytes;
		}
		mutex.unlock();
	}
}


int SerialCom::open(int aSerialPortNum, int aBoundRate)
{
	std::cout << "シリアルポートを開いています..." << std::endl;
	if (mComState == OPEN)
	{
		std::cerr << "現在開いているポートを閉じてください." << std::endl;
		return -1;
	}

	mSerialPortNum = aSerialPortNum;
	mBoundRate = aBoundRate;

	/* COMポートを開く */
	std::string portName("COM");
	portName += std::to_string(aSerialPortNum);
	if ((mPortHandle = CreateFile((LPCTSTR)(portName.c_str()),
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, 0, NULL))
		== INVALID_HANDLE_VALUE)
	{
		std::cerr << "ポートを開くことができません.ポート番号を確認してください." << std::endl;
		DWORD errorCode = GetLastError();
		std::cerr << errorCode << std::endl;
		return -1;
	}

	/* ボーレートの設定 */
	DCB config;
	GetCommState(mPortHandle, &config);
	config.BaudRate = mBoundRate;
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
	while (count >= aTxDataSize)
	{
		WriteFile(mPortHandle, aTxData, aTxDataSize - count, (LPDWORD)(&wroteBytes), NULL);
		count += wroteBytes;
	}
	return 0;
}


int SerialCom::sendChar(unsigned char aTxChar)
{
	mutex.lock();
	int wroteByte = 0;
	while (wroteByte == 0)
	{
		WriteFile(mPortHandle, &aTxChar, 1, (LPDWORD)(&wroteByte), NULL);
	}
	mutex.unlock();
	return 0;
}


int SerialCom::readRxBuffer(unsigned char *aReadBuffer, int aReadBufferSize)
{
	int toReadDataSize;
	int readSize;
	mutex.lock();

	if (mRxBufLength == 0)
	{
		mutex.unlock();
		return 0;
	}
	memcpy_s(aReadBuffer, aReadBufferSize, mRxBuffer, mRxBufLength);
	int length = mRxBufLength;
	mRxBufLength = 0;
	mutex.unlock();
	return length;
}


int SerialCom::clearRxBuffer()
{
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
