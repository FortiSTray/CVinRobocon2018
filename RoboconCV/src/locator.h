#ifndef _LOCATOR_H
#define _LOCATOR_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define REGULAR_QRCODE_SIDE 50
#define STANDBY_MARKER_NUM 10

typedef RotatedRect Marker;

typedef struct
{
	Mat image;
	bool lable;
}QRCode;

class Locator
{
public:
	Locator(void);
	~Locator(void);

	//QRCode定位主函数
	QRCode locate(Mat &img);

private:
	//从三个marker的信息中获取QRCode
	void getQRCode(vector<Marker> &markerSet);

	//寻找右下角的虚拟marker中心点
	Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT);

	//寻找一个点集中距离一条直线最远的点
	Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB);

	//从预备 Marker 的集合里面找出 真·Marker
	vector<Marker> findMarkerReal(Marker* standbyMarker, int MarkerCnt);

	//两点间距离计算
	inline float calcDistance(Point2f pointA, Point2f pointB)
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

		//计算直线的三个参数
		a = linePointA.y - linePointB.y;
		b = linePointB.x - linePointA.x;
		c = linePointA.x * linePointB.y - linePointB.x * linePointA.y;

		dist = fabsf(a * point.x + b * point.y + c) / sqrtf(a * a + b * b);
		return dist;
	}

private:
	//过程图像定义
	Mat srcImage;
	Mat preProcImage;
	Mat debugImage;
	Mat testImage;

	//轮廓定义
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	
	//预备 Marker
	Marker standbyMarker[STANDBY_MARKER_NUM];
	int standbyMarkerCnt;

	//定位标记集合定义
	vector<Marker> markerSet;

	//QRCode Marker
	Marker markerLeftTop;
	Marker markerLeftBottom;
	Marker markerRightTop;

	//QRCode 顶点
	Point2f cornerLeftTop;
	Point2f cornerLeftBottom;
	Point2f cornerRightTop;

	//定位到的目标QRCode
	QRCode dstQRCode;
};
#endif