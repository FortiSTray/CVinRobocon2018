#include "decoder.h"
#include "locator.h"

Decoder::Decoder(void)
{

}

Decoder::~Decoder(void)
{

}

int Decoder::decode(Mat& img)
{
	int grayscale;
	int pixNum;
	
	srcImage = img.clone();

	cvtColor(srcImage, preProcImage, COLOR_BGR2GRAY);
	threshold(preProcImage, preProcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	imshow("Processed Signal", preProcImage);
	
	//左上信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_LU_Y; i < MESSAGE_LU_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_LU_X; j < MESSAGE_LU_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageLUStatus = 1;
	}
	else
	{
		messageLUStatus = 0;
	}

	//中上信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_MU_Y; i < MESSAGE_MU_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_MU_X; j < MESSAGE_MU_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageMUStatus = 1;
	}
	else
	{
		messageMUStatus = 0;
	}

	//右上信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_RU_Y; i < MESSAGE_RU_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_RU_X; j < MESSAGE_RU_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageRUStatus = 1;
	}
	else
	{
		messageRUStatus = 0;
	}

	//左下信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_LD_Y; i < MESSAGE_LD_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_LD_X; j < MESSAGE_LD_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageLDStatus = 1;
	}
	else
	{
		messageLDStatus = 0;
	}

	//中下信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_MD_Y; i < MESSAGE_MD_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_MD_X; j < MESSAGE_MD_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageMDStatus = 1;
	}
	else
	{
		messageMDStatus = 0;
	}

	//右下信息块
	grayscale = 0;
	pixNum = 0;
	for (int i = MESSAGE_RD_Y; i < MESSAGE_RD_Y + MESSAGE_SIZE; i += 3)
	{
		for (int j = MESSAGE_RD_X; j < MESSAGE_RD_X + MESSAGE_SIZE; j += 3)
		{
			grayscale += preProcImage.ptr<uchar>(i)[j];
			pixNum++;
		}
	}
	grayscale /= pixNum;
	if (grayscale < 128)
	{
		messageRDStatus = 1;
	}
	else
	{
		messageRDStatus = 0;
	}

	//judge signal
	cout << messageLUStatus << " " << messageMUStatus << " " << messageRUStatus << " " <<
		messageLDStatus << " " << messageMDStatus << " " << messageRDStatus << " " << endl;

	return -1;
}