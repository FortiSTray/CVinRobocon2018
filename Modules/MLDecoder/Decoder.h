#pragma once
#include <string>
#include<iostream>

#include <mclmcr.h>
#include <matrix.h>
#include <mclcppclass.h>
#include "DecoderLabel.h"

#include "opencv\highgui.h"
#include "opencv2\opencv.hpp"

using namespace cv;

class Decoder
{
public:
	static bool init();
	static Decoder* getInstance();

	int predictData(double*roi);
	int predictData(double*roi, int*buffer, int numofMat);
	int predictData(Mat&roi);



	int TestLoadData();
private:
	static Decoder *decoder;
	Decoder();
	~Decoder();
};

