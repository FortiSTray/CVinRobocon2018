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
	vector<Point> contour;     //Position Pattern ������
	RotatedRect outerRect;     //�������С��ת����
}PositionPattern;

typedef struct
{

}QRCode;

class Locator
{
public:
	Locator(void);
	~Locator(void);

	//��λ������
	Mat locate(Mat &img);

	//test----------------------------------------------------------
	Mat locateByContours(Mat &img);
	//--------------------------------------------------------------

private:
	//��ȡ���򻯵�ROI
	Mat getRegularROI(Mat &img, RotatedRect &rect, int ROISide);

	//��X��Y�����ж��Ƿ�ΪPosition Pattern
	bool judgePositionPatternByX(Mat &img);
	bool judgePositionPatternByY(Mat &img);

	//��ȡQRCode
	void getQRCode(vector<PositionPattern> posiPatterns);

	//test----------------------------------------------------------
	//--------------------------------------------------------------

	//�����������
	inline double calcDistant(Point2f pointA, Point2f pointB)
	{
		double xDifference = pointA.x - pointB.x;
		double yDifference = pointA.y - pointB.y;
		return sqrt(xDifference * xDifference + yDifference * yDifference);
	}

	//��������������������
	inline double calcCrossProduct(Vec2f vectorA, Vec2f vectorB)
	{

	}

private:
	Mat srcImage;
	Mat positionPatternImage;
	Mat locationImage;
	Mat QRCodeROI;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<PositionPattern> positionPatterns;
	vector<Point> QRLocateContour;
	QRCode targetQRCode;
};
#endif