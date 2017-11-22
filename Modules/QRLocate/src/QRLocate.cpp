#include "QRLocate.h"

#define REGULAR_SIDE 70

//获取ROI -> 旋转 -> 裁切 -> 尺寸调整 -> 正则化的图像
Mat GetRegularROI(Mat &img, RotatedRect &rect)
{
	float diagonal = sqrt(pow(rect.size.width, 2) + pow(rect.size.height, 2));
	float ROIX = rect.center.x - diagonal / 2.0f;
	float ROIY = rect.center.y - diagonal / 2.0f;

	//防止越界
	if (ROIX < 0 || ROIY < 0 || ROIX + diagonal >= img.cols || ROIY + diagonal >= img.rows)
	{
		return Mat::zeros(1, 1, CV_32FC1);
	}

	Mat ROIImage = img(Rect(static_cast<int>(ROIX), static_cast<int>(ROIY), 
							static_cast<int>(diagonal), static_cast<int>(diagonal)));
	Mat rotateImage = Mat::zeros(static_cast<int>(diagonal), static_cast<int>(diagonal), CV_32FC1);
	Point2f ROICenter(diagonal / 2.0f, diagonal / 2.0f);
	Mat regularImage = Mat::zeros(REGULAR_SIDE, REGULAR_SIDE, CV_32FC1);

	//仿射变换
	Mat rotateMatrix = getRotationMatrix2D(ROICenter, rect.angle, 1);
	warpAffine(ROIImage, rotateImage, rotateMatrix, Size(static_cast<int>(diagonal), static_cast<int>(diagonal)));

	//裁切掉多余图像
	rotateImage = rotateImage(Rect(static_cast<int>(2.0f + (diagonal - rect.size.width) / 2.0f), 
									static_cast<int>(2.0f + (diagonal - rect.size.height) / 2.0f),
									static_cast<int>(rect.size.width), static_cast<int>(rect.size.height)));

	//尺寸正则化
	resize(rotateImage, regularImage, Size(REGULAR_SIDE, REGULAR_SIDE));
	threshold(regularImage, regularImage, 0, 255, THRESH_BINARY | THRESH_OTSU);

	//imshow("ROIImage", regularImage);
	//waitKey(0);

	return regularImage;
}

bool JudgeCornerByX(Mat &img)
{
	int totalPixCnt[5] = { 0 };
	int validPixCnt[5] = { 0 };
	vector<double> ratioCalc;
	vector<double> stdRatio = { 1.0f, 1.0f, 3.0f, 1.0f, 1.0f };
	ratioCalc.clear();

	if (img.rows != REGULAR_SIDE || img.cols != REGULAR_SIDE)
	{
		return false;
	}

	//计算总像素数和有效像素数
#define _pix_cnt_(value, n) do \
							{ \
								totalPixCnt[n]++; \
								if (img.ptr<uchar>(i)[j] == value) { validPixCnt[n]++; } \
							} while (0) \

	for (int i = REGULAR_SIDE / 7 * 2; i < REGULAR_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_SIDE / 7 * 0; j < REGULAR_SIDE / 7 * 1; j++) { _pix_cnt_(0  , 0); }
		for (int j = REGULAR_SIDE / 7 * 1; j < REGULAR_SIDE / 7 * 2; j++) { _pix_cnt_(255, 1); }
		for (int j = REGULAR_SIDE / 7 * 2; j < REGULAR_SIDE / 7 * 5; j++) { _pix_cnt_(0  , 2); }
		for (int j = REGULAR_SIDE / 7 * 5; j < REGULAR_SIDE / 7 * 6; j++) { _pix_cnt_(255, 3); }
		for (int j = REGULAR_SIDE / 7 * 6; j < REGULAR_SIDE / 7 * 7; j++) { _pix_cnt_(0  , 4); }
	}
	if (validPixCnt[2] == 0) { return false; }

#undef _pix_cnt_

	//计算有效像素 / 总像素, 比值过低则返回false
	for (int i = 0; i < 5; i++)
	{
		float validRatio = static_cast<float>(validPixCnt[i]) / static_cast<float>(totalPixCnt[i]);
		if (validRatio <= 0.6f)
		{
			return false;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		double ratio = static_cast<double>(validPixCnt[i]) / static_cast<double>(validPixCnt[2] / 3.0f);
		ratioCalc.push_back(ratio);
	}

#define ERROR_RANGE 0.6f

	if (fabs(ratioCalc[0] - stdRatio[0]) <= ERROR_RANGE && fabs(ratioCalc[1] - stdRatio[1]) <= ERROR_RANGE &&
		fabs(ratioCalc[2] - stdRatio[2]) <= ERROR_RANGE && fabs(ratioCalc[3] - stdRatio[3]) <= ERROR_RANGE &&
		fabs(ratioCalc[4] - stdRatio[4]) <= ERROR_RANGE)
	{
		return true;
	}
	else { return false; }

#undef ERROR_RANGE
}

bool JudgeCornerByY(Mat &img)
{
	int totalPixCnt[5] = { 0 };
	int validPixCnt[5] = { 0 };
	vector<double> ratioCalc;
	vector<double> stdRatio = { 1.0f, 1.0f, 3.0f, 1.0f, 1.0f };
	ratioCalc.clear();

	if (img.rows != REGULAR_SIDE || img.cols != REGULAR_SIDE)
	{
		return false;
	}

	//计算总像素数和有效像素数
#define _pix_cnt_(value, n) do \
							{ \
								totalPixCnt[n]++; \
								if (img.ptr<uchar>(j)[i] == value) { validPixCnt[n]++; } \
							} while (0) \

	for (int i = REGULAR_SIDE / 7 * 2; i < REGULAR_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_SIDE / 7 * 0; j < REGULAR_SIDE / 7 * 1; j++) { _pix_cnt_(0, 0); }
		for (int j = REGULAR_SIDE / 7 * 1; j < REGULAR_SIDE / 7 * 2; j++) { _pix_cnt_(255, 1); }
		for (int j = REGULAR_SIDE / 7 * 2; j < REGULAR_SIDE / 7 * 5; j++) { _pix_cnt_(0, 2); }
		for (int j = REGULAR_SIDE / 7 * 5; j < REGULAR_SIDE / 7 * 6; j++) { _pix_cnt_(255, 3); }
		for (int j = REGULAR_SIDE / 7 * 6; j < REGULAR_SIDE / 7 * 7; j++) { _pix_cnt_(0, 4); }
	}
	if (validPixCnt[2] == 0) { return false; }

#undef _pix_cnt_

	//计算有效像素 / 总像素, 比值过低则返回false
	for (int i = 0; i < 5; i++)
	{
		float validRatio = static_cast<float>(validPixCnt[i]) / static_cast<float>(totalPixCnt[i]);
		if (validRatio <= 0.6f)
		{ 
			return false;
		}
	}

	for (int i = 0; i < 5; i++)
	{
		double ratio = static_cast<double>(validPixCnt[i]) / static_cast<double>(validPixCnt[2] / 3.0f);
		ratioCalc.push_back(ratio);
	}

#define ERROR_RANGE 0.6f

	if (fabs(ratioCalc[0] - stdRatio[0]) <= ERROR_RANGE && fabs(ratioCalc[1] - stdRatio[1]) <= ERROR_RANGE &&
		fabs(ratioCalc[2] - stdRatio[2]) <= ERROR_RANGE && fabs(ratioCalc[3] - stdRatio[3]) <= ERROR_RANGE &&
		fabs(ratioCalc[4] - stdRatio[4]) <= ERROR_RANGE)
	{
		return true;
	}
	else { return false; }

#undef ERROR_RANGE
}