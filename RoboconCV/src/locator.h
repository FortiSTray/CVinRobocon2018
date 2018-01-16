#ifndef _LOCATOR_H
#define _LOCATOR_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define REGULAR_SIGNAL_HEIGHT 21
#define REGULAR_SIGNAL_WIDTH  75

#define POSSIBLE_MARKER_NUM 10

typedef struct
{
	Point cornerLeftTop;
	Point cornerLeftBottom;
	Point cornerRightTop;
	Point cornerRightBottom;
}Marker;

typedef struct
{
	Mat image;
	bool lable;
}Signal;

class Locator
{
public:
	Locator(void);
	~Locator(void);

	//Signal定位主函数
	Signal locate(Mat &img);

private:
	//从可能的Marker里面找出最终的Marker对
	vector<Marker> findMarkerPair(Marker* psbMarker, int MarkerCnt);

	//从三个marker的信息中获取Signal
	Signal getSignal(vector<Marker> markerPair);

	//两点间距离计算
	inline float calcDistance(Point2f pointA, Point2f pointB)
	{
		float xDifference = pointA.x - pointB.x;
		float yDifference = pointA.y - pointB.y;
		return sqrt(xDifference * xDifference + yDifference * yDifference);
	}

	//两点中点计算
	inline Point2f calcMidpoint(Point2f pointA, Point2f pointB)
	{
		Point2f midpoint;

		midpoint.x = (pointA.x + pointB.x) / 2.0f;
		midpoint.y = (pointA.y + pointB.y) / 2.0f;
		return midpoint;
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

	//计算点集的重心
	inline Point2f calcGravityCenter(vector<Point> points)
	{
		Point2f gravityCenter;

		for (auto crtPoint : points)
		{
			gravityCenter.x += crtPoint.x;
			gravityCenter.y += crtPoint.y;
		}
		gravityCenter.x /= points.size();
		gravityCenter.y /= points.size();

		return gravityCenter;
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
	
	//可能的Marker
	Marker psbMarker[POSSIBLE_MARKER_NUM];
	int psbMarkerCnt;

	//用于标志定位的Marker对
	vector<Marker> markerPair;

	//定位到的目标Signal
	Signal dstSignal;
};
#endif