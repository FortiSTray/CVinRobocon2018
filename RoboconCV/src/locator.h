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
	vector<Point> contour;     //Position Pattern 的轮廓
	RotatedRect outerRect;     //外包括最小旋转矩形
}PositionPattern;

typedef struct
{

}QRCode;

class Locator
{
public:
	Locator(void);
	~Locator(void);

	//定位主函数
	Mat locate(Mat &img);

	//test----------------------------------------------------------
	Mat locateByContours(Mat &img);
	//--------------------------------------------------------------

private:
	//获取正则化的ROI
	Mat getRegularROI(Mat &img, RotatedRect &rect, int ROISide);

	//从X与Y方向判断是否为Position Pattern
	bool judgePositionPatternByX(Mat &img);
	bool judgePositionPatternByY(Mat &img);

	//获取QRCode
	void getQRCode(vector<PositionPattern> posiPatterns);

	//test----------------------------------------------------------
	//--------------------------------------------------------------

	//两点间距离计算
	inline double calcDistant(Point2f pointA, Point2f pointB)
	{
		double xDifference = pointA.x - pointB.x;
		double yDifference = pointA.y - pointB.y;
		return sqrt(xDifference * xDifference + yDifference * yDifference);
	}

	//计算两个向量的向量积
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