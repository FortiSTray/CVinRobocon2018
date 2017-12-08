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

void getQRCode(vector<PositionPattern> &posiPatterns)
{
	if (posiPatterns.size() == (size_t)3)
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
		Point pointVirtual;

		RotatedRect rectLeftTop;
		RotatedRect rectLeftBottom;
		RotatedRect rectRightTop;

		Point pointTemp;

		Point2f vectorA;
		Point2f vectorB;

		Point2f tempCorners[4];

		Point cornerLeftTop;
		Point cornerLeftBottom;
		Point cornerRightTop;

		Point2f srcCorner[3];
		Point2f dstCorner[3];

		float tempDist;

		rectLeftTop = posiPatterns[0].outerRect;

		//找出三个点
		if (distantAB > distantBC && distantAB > distantAC)
		{
			pointLeftTop = pointC;
			rectLeftTop = posiPatterns[2].outerRect;

			vectorA.x = pointA.x - pointLeftTop.x;
			vectorA.y = pointA.y - pointLeftTop.y;
			vectorB.x = pointB.x - pointLeftTop.x;
			vectorB.y = pointB.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointB;
				pointRightTop = pointA;
				rectLeftBottom = posiPatterns[1].outerRect;
				rectRightTop = posiPatterns[0].outerRect;
			}
			else
			{
				pointLeftBottom = pointA;
				pointRightTop = pointB;
				rectLeftBottom = posiPatterns[0].outerRect;
				rectRightTop = posiPatterns[1].outerRect;
			}
		}
		else if (distantBC > distantAB && distantBC > distantAC)
		{
			pointLeftTop = pointA;
			rectLeftTop = posiPatterns[0].outerRect;

			vectorA.x = pointB.x - pointLeftTop.x;
			vectorA.y = pointB.y - pointLeftTop.y;
			vectorB.x = pointC.x - pointLeftTop.x;
			vectorB.y = pointC.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointC;
				pointRightTop = pointB;
				rectLeftBottom = posiPatterns[2].outerRect;
				rectRightTop = posiPatterns[1].outerRect;
			}
			else
			{
				pointLeftBottom = pointB;
				pointRightTop = pointC;
				rectLeftBottom = posiPatterns[1].outerRect;
				rectRightTop = posiPatterns[2].outerRect;
			}
		}
		else if (distantAC > distantAB && distantAC > distantBC)
		{
			pointLeftTop = pointB;
			rectLeftTop = posiPatterns[1].outerRect;

			vectorA.x = pointA.x - pointLeftTop.x;
			vectorA.y = pointA.y - pointLeftTop.y;
			vectorB.x = pointC.x - pointLeftTop.x;
			vectorB.y = pointC.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointC;
				pointRightTop = pointA;
				rectLeftBottom = posiPatterns[2].outerRect;
				rectRightTop = posiPatterns[0].outerRect;
			}
			else
			{
				pointLeftBottom = pointA;
				pointRightTop = pointC;
				rectLeftBottom = posiPatterns[0].outerRect;
				rectRightTop = posiPatterns[2].outerRect;
			}
		}
		else {}
		
		//找出右下虚拟点
		pointVirtual = findPointVirtual(pointLeftTop, pointLeftBottom, pointRightTop);

		//找出二维码的三个角
		rectLeftTop.points(tempCorners);
		cornerLeftTop = findFarthestPoint(tempCorners, pointLeftBottom, pointRightTop);

		rectLeftBottom.points(tempCorners);
		cornerLeftBottom = findFarthestPoint(tempCorners, pointLeftTop, pointVirtual);

		rectRightTop.points(tempCorners);
		cornerRightTop = findFarthestPoint(tempCorners, pointLeftTop, pointVirtual);

		Mat temp = imread("QRCode.jpg");
		Mat output;
		circle(temp, cornerLeftTop, 3, Scalar(255, 255, 0));
		circle(temp, cornerLeftBottom, 3, Scalar(0, 255, 0));
		circle(temp, cornerRightTop, 3, Scalar(0, 0, 255));
		imshow("point", temp);

		srcCorner[0] = cornerLeftTop;
		srcCorner[1] = cornerLeftBottom;
		srcCorner[2] = cornerRightTop;

		dstCorner[0] = Point(0, 0);
		dstCorner[1] = Point(0, 50);
		dstCorner[2] = Point(50, 0);

		Mat affineMatrix;
		affineMatrix = getAffineTransform(srcCorner, dstCorner);
		warpAffine(temp, output, affineMatrix, Size(50, 50));
		imshow("QR", output);
	}
}

//两点间距离计算
double calcDistant(Point2f pointA, Point2f pointB)
{
	double xDifference = pointA.x - pointB.x;
	double yDifference = pointA.y - pointB.y;
	return sqrt(xDifference * xDifference + yDifference * yDifference);
}

double calcCrossProduct(Point2f vectorA, Point2f vectorB)
{
	return vectorA.x * vectorB.y - vectorA.y * vectorB.x;
}

Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT)
{
	Point2f vectorA;
	Point2f vectorB;
	Point2f vectorTarget;
	Point2f pointVirtual;

	vectorA.x = pointLB.x - pointLT.x;
	vectorA.y = pointLB.y - pointLT.y;
	vectorB.x = pointRT.x - pointLT.x;
	vectorB.y = pointRT.y - pointLT.y;

	vectorTarget.x = vectorA.x + vectorB.x;
	vectorTarget.y = vectorA.y + vectorB.y;

	pointVirtual.x = pointLT.x + vectorTarget.x;
	pointVirtual.y = pointLT.y + vectorTarget.y;

	return pointVirtual;
}

double distPointToLine(Point2f point, Point2f linePointA, Point2f linePointB)
{
	double a, b, c;
	double dist;

	a = linePointA.y - linePointB.y;
	b = -(linePointA.x - linePointB.x);
	c = linePointA.x * linePointB.y - linePointB.x * linePointA.y;

	dist = fabsf(a * point.x + b * point.y + c) / sqrt(a * a + b * b);
	return dist;
}

Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB)
{
	float tempDist = 0.0f;
	float farthestDist = 0.0f;
	Point2f farthestPoint;

	for (size_t i = 0; i < 4; i++)
	{
		tempDist = distPointToLine(pointSet[i], linePointA, linePointB);
		if (tempDist > farthestDist)
		{
			farthestDist = tempDist;
			farthestPoint = pointSet[i];
		}
	}

	return farthestPoint;
}