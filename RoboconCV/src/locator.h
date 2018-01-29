#ifndef _LOCATOR_H
#define _LOCATOR_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define Pi 3.14159265359f

#define REGULAR_SIGNAL_HEIGHT 21
#define REGULAR_SIGNAL_WIDTH  75

#define POSSIBLE_MARKER_NUM 10

typedef struct
{
	Point cornerLeftTop;
	Point cornerLeftBottom;
	Point cornerRightTop;
	Point cornerRightBottom;

	//重载[]运算符以用序号访问四个角点
	Point operator[](uchar i)
	{
		if (i == 0)        { return cornerLeftTop; }
		else if (i == 1)   { return cornerLeftBottom; }
		else if (i == 2)   { return cornerRightTop; }
		else if (i == 3)   { return cornerRightBottom; }
		else               { cout << "Marker corner access denied!" << endl; return Point(0, 0); }
	}
}Marker;

typedef struct
{
	Mat image;
	bool lable;      //检测到Signal则lable置true，否则置false
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

	//从Marker对的信息中获取目标Signal
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

	//计算两个向量所在直线的夹角（锐角）
	inline float calcSharpAngle(Point2f vectorA, Point2f vectorB)
	{
		float dotProduct = vectorA.x * vectorB.x + vectorA.y * vectorB.y;
		float modA = sqrtf(vectorA.x * vectorA.x + vectorA.y * vectorA.y);
		float modB = sqrtf(vectorB.x * vectorB.x + vectorB.y * vectorB.y);

		float sharpAngle = acos(dotProduct / (modA * modB));
		if (sharpAngle >= 0 && sharpAngle <= Pi / 2.0f)
		{
			return sharpAngle;
		}
		else
		{
			return Pi - sharpAngle;
		}
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
#endif //_LOCATOR_H