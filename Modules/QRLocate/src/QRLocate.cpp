#include "QRLocate.h"

#define REGULAR_SIDE 49

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

	Mat ROIImage = img(Rect(ROIX, ROIY, diagonal, diagonal));
	Mat rotateImage = Mat::zeros(diagonal, diagonal, CV_32FC1);
	Point2f ROICenter(diagonal / 2.0f, diagonal / 2.0f);
	Mat regularImage = Mat::zeros(REGULAR_SIDE, REGULAR_SIDE, CV_32FC1);

	//仿射变换
	Mat rotateMatrix = getRotationMatrix2D(ROICenter, rect.angle, 1);
	warpAffine(ROIImage, rotateImage, rotateMatrix, Size(diagonal, diagonal));

	//裁切掉多余图像
	rotateImage = rotateImage(Rect(2 + (diagonal - rect.size.width) / 2, 2 + (diagonal - rect.size.height) / 2,
		rect.size.width, rect.size.height));

	//尺寸正则化
	resize(rotateImage, regularImage, Size(REGULAR_SIDE, REGULAR_SIDE));
	threshold(regularImage, regularImage, 0, 255, THRESH_BINARY | THRESH_OTSU);

	imshow("ROIImage", regularImage);
	waitKey(0);

	return regularImage;
}

bool JudgeCornerByX(Mat &img)
{
	int pixCounter[5] = { 0 };
	vector<int> ratioCalc;
	ratioCalc.clear();

	if (img.rows != REGULAR_SIDE || img.cols != REGULAR_SIDE)
	{
		cout << "Input image not regular." << endl;
		return false;
	}

	for (int i = REGULAR_SIDE / 7 * 2; i < REGULAR_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_SIDE / 7 * 0; j < REGULAR_SIDE / 7 * 1; j++) { if (img.ptr<uchar>(i)[j] == 0) { pixCounter[0]++; } }
		if (pixCounter[0] == 0) { return false; }
		for (int j = REGULAR_SIDE / 7 * 1; j < REGULAR_SIDE / 7 * 2; j++) { if (img.ptr<uchar>(i)[j] == 255) { pixCounter[1]++; } }
		if (pixCounter[0] == 1) { return false; }
		for (int j = REGULAR_SIDE / 7 * 2; j < REGULAR_SIDE / 7 * 5; j++) { if (img.ptr<uchar>(i)[j] == 0) { pixCounter[2]++; } }
		if (pixCounter[0] == 2) { return false; }
		for (int j = REGULAR_SIDE / 7 * 5; j < REGULAR_SIDE / 7 * 6; j++) { if (img.ptr<uchar>(i)[j] == 255) { pixCounter[3]++; } }
		if (pixCounter[0] == 3) { return false; }
		for (int j = REGULAR_SIDE / 7 * 6; j < REGULAR_SIDE / 7 * 7; j++) { if (img.ptr<uchar>(i)[j] == 0) { pixCounter[4]++; } }
		if (pixCounter[0] == 4) { return false; }
	}

	for (int i = 0; i < 5; i++)
	{
		int x = static_cast<int>(static_cast<double>(pixCounter[i]) / static_cast<double>(pixCounter[0]) + 0.5);
		ratioCalc.push_back(x);
	}

	if (ratioCalc[0] == 1 && ratioCalc[1] == 1 && ratioCalc[2] == 3 && ratioCalc[3] == 1 && ratioCalc[4] == 1)
	{
		return true;
	}
	else { return false; }
}
bool JudgeCornerByY(Mat &img)
{
	int pixCounter[5] = { 0 };
	vector<int> ratioCalc;
	ratioCalc.clear();

	if (img.rows != REGULAR_SIDE || img.cols != REGULAR_SIDE)
	{
		cout << "Input image not regular." << endl;
		return false;
	}

	for (int i = REGULAR_SIDE / 7 * 2; i < REGULAR_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_SIDE / 7 * 0; j < REGULAR_SIDE / 7 * 1; j++) { if (img.ptr<uchar>(j)[i] == 0) { pixCounter[0]++; } }
		if (pixCounter[0] == 0) { return false; }
		for (int j = REGULAR_SIDE / 7 * 1; j < REGULAR_SIDE / 7 * 2; j++) { if (img.ptr<uchar>(j)[i] == 255) { pixCounter[1]++; } }
		if (pixCounter[0] == 1) { return false; }
		for (int j = REGULAR_SIDE / 7 * 2; j < REGULAR_SIDE / 7 * 5; j++) { if (img.ptr<uchar>(j)[i] == 0) { pixCounter[2]++; } }
		if (pixCounter[0] == 2) { return false; }
		for (int j = REGULAR_SIDE / 7 * 5; j < REGULAR_SIDE / 7 * 6; j++) { if (img.ptr<uchar>(j)[i] == 255) { pixCounter[3]++; } }
		if (pixCounter[0] == 3) { return false; }
		for (int j = REGULAR_SIDE / 7 * 6; j < REGULAR_SIDE / 7 * 7; j++) { if (img.ptr<uchar>(j)[i] == 0) { pixCounter[4]++; } }
		if (pixCounter[0] == 4) { return false; }
	}

	for (int i = 0; i < 5; i++)
	{
		int x = static_cast<int>(static_cast<double>(pixCounter[i]) / static_cast<double>(pixCounter[0]) + 0.5);
		ratioCalc.push_back(x);
	}

	if (ratioCalc[0] == 1 && ratioCalc[1] == 1 && ratioCalc[2] == 3 && ratioCalc[3] == 1 && ratioCalc[4] == 1)
	{
		return true;
	}
	else { return false; }
}