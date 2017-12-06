#include "locator.h"

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	positionPatterns.erase(positionPatterns.begin(), positionPatterns.end());
}

Mat Locator::locate(Mat &img)
{
	PositionPattern tempPositionPattern;
	positionPatterns.erase(positionPatterns.begin(), positionPatterns.end());
	QRLocateContour.erase(QRLocateContour.begin(), QRLocateContour.end());

	srcImage = img.clone();
	positionPatternImage = img.clone();

	namedWindow("Src", CV_WINDOW_AUTOSIZE);
	imshow("Src", srcImage);

	cvtColor(srcImage, srcImage, COLOR_BGR2GRAY);
	medianBlur(srcImage, srcImage, 3);
	threshold(srcImage, srcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//adaptiveThreshold(srcImage, srcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 55, 0);
	imshow("Threshold", srcImage);

	//detect rectangle now
	findContours(srcImage.clone(), contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());

	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if (area < 100) continue;
		RotatedRect rect = minAreaRect(contours[i]);

		//根据矩形特征进行几何分析
		float rectWidth = rect.size.width;
		float rectHeight = rect.size.height;
		float ratio = min(rectWidth, rectHeight) / max(rectWidth, rectHeight);

		if (ratio > 0.85 && rectWidth < srcImage.cols / 4 && rectHeight < srcImage.rows / 4)
		{
			Mat regularROI = getRegularROI(srcImage, rect, REGULAR_CORNER_SIDE);

			if (judgePositionPatternByX(regularROI) && judgePositionPatternByY(regularROI))
			{
				drawContours(positionPatternImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);

				tempPositionPattern.contour = contours[i];
				tempPositionPattern.outerRect = rect;
				positionPatterns.push_back(tempPositionPattern);

				//imshow("roiImage", regularROI);
				//waitKey(0);
			}
		}
	}

	//QRLocateContour = getQRLocateContour(positionPatternContours);
	for (size_t i = 0; i < positionPatterns.size(); i++)
	{
		QRLocateContour.insert(QRLocateContour.end(), positionPatterns[i].contour.begin(), positionPatterns[i].contour.end());
	}

	//resize(locateResult, locateResult, Size(700, 700));
	//resize(locateResult, locateResult, Size(525, 700));
	if (positionPatterns.size() == (size_t)3)
	{
		RotatedRect rectQRCode = minAreaRect(QRLocateContour);
		QRCodeROI = getRegularROI(srcImage, rectQRCode, REGULAR_QRCODE_SIDE);
		imshow("result", positionPatternImage);
		imshow("ROI", QRCodeROI);
	}

	return QRCodeROI;
}

//获取ROI -> 旋转 -> 裁切 -> 尺寸调整 -> 正则化的图像
Mat Locator::getRegularROI(Mat &img, RotatedRect &rect, int ROISide)
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
	Mat regularImage = Mat::zeros(ROISide, ROISide, CV_32FC1);

	//仿射变换
	Mat rotateMatrix = getRotationMatrix2D(ROICenter, rect.angle, 1);
	warpAffine(ROIImage, rotateImage, rotateMatrix, Size(static_cast<int>(diagonal), static_cast<int>(diagonal)));

	//裁切掉多余图像
	rotateImage = rotateImage(Rect(static_cast<int>(2.0f + (diagonal - rect.size.width) / 2.0f),
		static_cast<int>(2.0f + (diagonal - rect.size.height) / 2.0f),
		static_cast<int>(rect.size.width), static_cast<int>(rect.size.height)));

	//尺寸正则化
	resize(rotateImage, regularImage, Size(ROISide, ROISide));
	threshold(regularImage, regularImage, 0, 255, THRESH_BINARY | THRESH_OTSU);

	//imshow("ROIImage", regularImage);
	//waitKey(0);

	return regularImage;
}

bool Locator::judgePositionPatternByX(Mat &img)
{
	int totalPixCnt[5] = { 0 };
	int validPixCnt[5] = { 0 };
	vector<double> ratioCalc;
	vector<double> stdRatio = { 1.0f, 1.0f, 3.0f, 1.0f, 1.0f };
	ratioCalc.clear();

	if (img.rows != REGULAR_CORNER_SIDE || img.cols != REGULAR_CORNER_SIDE)
	{
		return false;
	}

	//计算总像素数和有效像素数
#define _pix_cnt_(value, n) do \
							{ \
								totalPixCnt[n]++; \
								if (img.ptr<uchar>(i)[j] == value) { validPixCnt[n]++; } \
							} while (0) \

	for (int i = REGULAR_CORNER_SIDE / 7 * 2; i < REGULAR_CORNER_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_CORNER_SIDE / 7 * 0; j < REGULAR_CORNER_SIDE / 7 * 1; j++) { _pix_cnt_(0, 0); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 1; j < REGULAR_CORNER_SIDE / 7 * 2; j++) { _pix_cnt_(255, 1); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 2; j < REGULAR_CORNER_SIDE / 7 * 5; j++) { _pix_cnt_(0, 2); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 5; j < REGULAR_CORNER_SIDE / 7 * 6; j++) { _pix_cnt_(255, 3); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 6; j < REGULAR_CORNER_SIDE / 7 * 7; j++) { _pix_cnt_(0, 4); }
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

bool Locator::judgePositionPatternByY(Mat &img)
{
	int totalPixCnt[5] = { 0 };
	int validPixCnt[5] = { 0 };
	vector<double> ratioCalc;
	vector<double> stdRatio = { 1.0f, 1.0f, 3.0f, 1.0f, 1.0f };
	ratioCalc.clear();

	if (img.rows != REGULAR_CORNER_SIDE || img.cols != REGULAR_CORNER_SIDE)
	{
		return false;
	}

	//计算总像素数和有效像素数
#define _pix_cnt_(value, n) do \
							{ \
								totalPixCnt[n]++; \
								if (img.ptr<uchar>(j)[i] == value) { validPixCnt[n]++; } \
							} while (0) \

	for (int i = REGULAR_CORNER_SIDE / 7 * 2; i < REGULAR_CORNER_SIDE / 7 * 5; i++)
	{
		for (int j = REGULAR_CORNER_SIDE / 7 * 0; j < REGULAR_CORNER_SIDE / 7 * 1; j++) { _pix_cnt_(0, 0); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 1; j < REGULAR_CORNER_SIDE / 7 * 2; j++) { _pix_cnt_(255, 1); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 2; j < REGULAR_CORNER_SIDE / 7 * 5; j++) { _pix_cnt_(0, 2); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 5; j < REGULAR_CORNER_SIDE / 7 * 6; j++) { _pix_cnt_(255, 3); }
		for (int j = REGULAR_CORNER_SIDE / 7 * 6; j < REGULAR_CORNER_SIDE / 7 * 7; j++) { _pix_cnt_(0, 4); }
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

void Locator::getQRCode(vector<PositionPattern> posiPatterns)
{
	if (positionPatterns.size() == (size_t)3)
	{
		Point pointA = posiPatterns[0].outerRect.center;
		Point pointB = posiPatterns[1].outerRect.center;
		Point pointC = posiPatterns[2].outerRect.center;
		float distantAB = calcDistant(pointA, pointB);
		float distantBC = calcDistant(pointB, pointC);
		float distantAC = calcDistant(pointA, pointC);
		Point pointLeftTop;
		Point pointLeftBottom;
		Point pointRightTop;

		if (distantAB > distantBC && distantAB > distantAC)
		{
			pointLeftTop = pointC;
		}
		else if (distantBC > distantAB && distantBC > distantAC)
		{
			pointLeftTop = pointA;
		}
		else if (distantAC > distantAB && distantAC > distantBC)
		{
			pointLeftTop = pointB;
		}
		else
		{

		}
	}
}

//----------------------------------------------------------
Mat Locator::locateByContours(Mat &img)
{
	PositionPattern tempPositionPattern;
	positionPatterns.erase(positionPatterns.begin(), positionPatterns.end());
	QRLocateContour.erase(QRLocateContour.begin(), QRLocateContour.end());

	srcImage = img.clone();
	positionPatternImage = img.clone();

	namedWindow("Src", CV_WINDOW_AUTOSIZE);
	imshow("Src", srcImage);

	cvtColor(srcImage, srcImage, COLOR_BGR2GRAY);
	//medianBlur(srcImage, srcImage, 3);
	//imshow("blur", srcImage);
	//threshold(srcImage, srcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	adaptiveThreshold(srcImage, srcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 55, 0);
	imshow("Threshold", srcImage);

	//detect rectangle now
	findContours(srcImage.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if (area < 100) continue;
		RotatedRect rect = minAreaRect(contours[i]);

		//根据矩形特征进行几何分析
		float rectWidth = rect.size.width;
		float rectHeight = rect.size.height;
		float ratio = min(rectWidth, rectHeight) / max(rectWidth, rectHeight);

		if (ratio > 0.85 && rectWidth < srcImage.cols / 4 && rectHeight < srcImage.rows / 4)
		{
			//drawContours(locateResult, contours, static_cast<int>(t), Scalar(255, 0, 0), 2, 8);
			//imshow("result", locateResult);
			//waitKey(0);

			int k = i;
			int layerCounter = 0;

			while (hierarchy[k][2] != -1)
			{
				k = hierarchy[k][2];
				layerCounter = layerCounter + 1;
				if (layerCounter >= 2)
				{
					drawContours(positionPatternImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);

					tempPositionPattern.contour = contours[i];
					tempPositionPattern.outerRect = rect;
					positionPatterns.push_back(tempPositionPattern);
				}
			}
		}
	}

	getQRCode(positionPatterns);
	for (size_t i = 0; i < positionPatterns.size(); i++)
	{
		QRLocateContour.insert(QRLocateContour.end(), positionPatterns[i].contour.begin(), positionPatterns[i].contour.end());
	}

	//resize(locateResult, locateResult, Size(700, 700));
	//resize(locateResult, locateResult, Size(525, 700));
	if (positionPatterns.size() == (size_t)3)
	{
		RotatedRect rectQRCode = minAreaRect(QRLocateContour);
		QRCodeROI = getRegularROI(srcImage, rectQRCode, REGULAR_QRCODE_SIDE);
		imshow("result", positionPatternImage);
		imshow("ROI", QRCodeROI);
	}

	return QRCodeROI;
}