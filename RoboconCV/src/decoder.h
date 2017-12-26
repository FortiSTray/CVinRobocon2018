#ifndef _DECODER_H
#define _DECODER_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

//宏定义四个信号块的尺寸和位置
#define SIGNAL_SIZE 10

#define SIGNAL_UP_X 20
#define SIGNAL_UP_Y 8

#define SIGNAL_RIGHT_X 32
#define SIGNAL_RIGHT_Y 20

#define SIGNAL_DOWN_X 20
#define SIGNAL_DOWN_Y 32

#define SIGNAL_LEFT_X 8
#define SIGNAL_LEFT_Y 20

class Decoder
{
public:
	Decoder(void);
	~Decoder(void);

	int decode(Mat &img);

private:
	Mat srcImage;
	Mat preProcImage;

	bool signalUpStatus;
	bool signalRightStatus;
	bool signalDownStatus;
	bool signalLeftStatus;
};
#endif