#include "Decoder.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#define PIC_SIZE 28
bool Decoder::init()
{
	loadDeepnet();
	if (!DecoderLabelInitialize()) {
		std::cout << "init false in load dll" << std::endl;
		return false;
	}
	decoder = new Decoder();
	if (decoder == nullptr) {
		std::cout << "init instance fail" << std::endl;
		return false;
	}
	std::cout << "init successful" << std::endl;
	return true;
}

Decoder * Decoder::getInstance()
{
	return decoder;
}

int Decoder::predictData(double * roi)
{
	return 0;
}

int Decoder::predictData(double * roi, int * buffer, int numofMat)
{
	return 0;
}

int Decoder::predictData(Mat & roi)
{
	Mat buffer;
	roi.convertTo(buffer, CV_64F, 1.0 / 255.0);

	mwArray predictArray(PIC_SIZE, PIC_SIZE);

	predictArray.SetData((double*)buffer.data,PIC_SIZE*PIC_SIZE);
		
	mwArray label;
	DecoderLabel(1, label, predictArray);

	mxDouble result[1];
	label.GetData(result, 1);
	
	return result[0];
}

int Decoder::TestLoadData()
{
	return 0;
}

Decoder::Decoder()
{
	
}

Decoder::~Decoder()
{
}
