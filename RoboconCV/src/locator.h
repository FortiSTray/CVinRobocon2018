#ifndef _LOCATOR_H
#define _LOCATOR_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define REGULAR_CORNER_SIDE 70
#define REGULAR_QRCODE_SIDE 28

typedef struct
{
	vector<Point> contour;        //marker 的轮廓
	RotatedRect outerRect;        //外包括最小旋转矩形
}Marker;


class Locator
{
public:
	Locator(void);
	~Locator(void);

	//定位主函数
	Mat locate(Mat &img);

private:

	void getQRCode(vector<Marker> &markerSet);

	Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT);

	Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB);

	//两点间距离计算
	inline float calcDistant(Point2f pointA, Point2f pointB)
	{
		float xDifference = pointA.x - pointB.x;
		float yDifference = pointA.y - pointB.y;
		return sqrt(xDifference * xDifference + yDifference * yDifference);
	}

	//计算两个向量的向量积
	inline float calcCrossProduct(Point2f vectorA, Point2f vectorB)
	{
		return vectorA.x * vectorB.y - vectorA.y * vectorB.x;
	}

	//点到两点确定的直线的距离
	inline float distPointToLine(Point2f point, Point2f linePointA, Point2f linePointB)
	{
		float a, b, c;
		float dist;

		a = linePointA.y - linePointB.y;
		b = -(linePointA.x - linePointB.x);
		c = linePointA.x * linePointB.y - linePointB.x * linePointA.y;

		dist = fabsf(a * point.x + b * point.y + c) / sqrt(a * a + b * b);
		return dist;
	}

private:
	Mat srcImage;
	Mat markerImage;
	Mat QRCodeROI;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Marker> markerSet;
};
#endif