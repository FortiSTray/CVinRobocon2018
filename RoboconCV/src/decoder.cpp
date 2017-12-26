#include "decoder.h"
#include "locator.h"

Decoder::Decoder(void)
{

}

Decoder::~Decoder(void)
{

}

int Decoder::decode(Mat &img)
{
	int grayscale;
	int pixNum;
	
	srcImage = img.clone();

	cvtColor(srcImage, preProcImage, COLOR_BGR2GRAY);
	threshold(preProcImage, preProcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	
	//上信号块
	grayscale = 0;
	pixNum = 0;
	for (int i = SIGNAL_UP_Y; i < SIGNAL_UP_Y + SIGNAL_SIZE; i += 3)
	{
		for (int j = SIGNAL_UP_X; j < SIGNAL_UP_X + SIGNAL_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		signalUpStatus = 1;
	}
	else
	{
		signalUpStatus = 0;
	}

	//右信号块
	grayscale = 0;
	pixNum = 0;
	for (int i = SIGNAL_RIGHT_Y; i < SIGNAL_RIGHT_Y + SIGNAL_SIZE; i += 3)
	{
		for (int j = SIGNAL_RIGHT_X; j < SIGNAL_RIGHT_X + SIGNAL_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		signalRightStatus = 1;
	}
	else
	{
		signalRightStatus = 0;
	}

	//下信号块
	grayscale = 0;
	pixNum = 0;
	for (int i = SIGNAL_DOWN_Y; i < SIGNAL_DOWN_Y + SIGNAL_SIZE; i += 3)
	{
		for (int j = SIGNAL_DOWN_X; j < SIGNAL_DOWN_X + SIGNAL_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		signalDownStatus = 1;
	}
	else
	{
		signalDownStatus = 0;
	}

	//左信号块
	grayscale = 0;
	pixNum = 0;
	for (int i = SIGNAL_LEFT_Y; i < SIGNAL_LEFT_Y + SIGNAL_SIZE; i += 3)
	{
		for (int j = SIGNAL_LEFT_X; j < SIGNAL_LEFT_X + SIGNAL_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		signalLeftStatus = 1;
	}
	else
	{
		signalLeftStatus = 0;
	}

	//judge signal
	if (signalUpStatus == 1 && signalRightStatus == 1 && signalDownStatus == 1 && signalLeftStatus == 1)
	{
		return 0;
	}
	else if (signalUpStatus == 1 && signalRightStatus == 0 && signalDownStatus == 0 && signalLeftStatus == 0)
	{
		return 1;
	}
	else if (signalUpStatus == 1 && signalRightStatus == 1 && signalDownStatus == 0 && signalLeftStatus == 0)
	{
		return 2;
	}
	else if (signalUpStatus == 0 && signalRightStatus == 1 && signalDownStatus == 0 && signalLeftStatus == 0)
	{
		return 3;
	}
	else if (signalUpStatus == 0 && signalRightStatus == 1 && signalDownStatus == 1 && signalLeftStatus == 0)
	{
		return 4;
	}
	else if (signalUpStatus == 0 && signalRightStatus == 0 && signalDownStatus == 1 && signalLeftStatus == 0)
	{
		return 5;
	}
	else if (signalUpStatus == 0 && signalRightStatus == 0 && signalDownStatus == 1 && signalLeftStatus == 1)
	{
		return 6;
	}
	else if (signalUpStatus == 0 && signalRightStatus == 0 && signalDownStatus == 0 && signalLeftStatus == 1)
	{
		return 7;
	}
	else if (signalUpStatus == 1 && signalRightStatus == 0 && signalDownStatus == 0 && signalLeftStatus == 1)
	{
		return 8;
	}
	else
	{
		return -1;
	}
}