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
	vector<Point> contour;        //marker ������
	RotatedRect outerRect;        //�������С��ת����
}Marker;


class Locator
{
public:
	Locator(void);
	~Locator(void);

	//��λ������
	Mat locate(Mat &img);

private:

	void getQRCode(vector<Marker> &markerSet);

	Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT);

	Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB);

	//�����������
	inline float calcDistant(Point2f pointA, Point2f pointB)
	{
		float xDifference = pointA.x - pointB.x;
		float yDifference = pointA.y - pointB.y;
		return sqrt(xDifference * xDifference + yDifference * yDifference);
	}

	//��������������������
	inline float calcCrossProduct(Point2f vectorA, Point2f vectorB)
	{
		return vectorA.x * vectorB.y - vectorA.y * vectorB.x;
	}

	//�㵽����ȷ����ֱ�ߵľ���
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