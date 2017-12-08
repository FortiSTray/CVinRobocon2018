#ifndef QR_LOCATE_H
#define QR_LOCATE_H

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


//detect rectangle now
typedef int orientationType;

typedef struct
{
	vector<Point> contour;        //Position Pattern 的轮廓
	RotatedRect outerRect;        //外包括最小旋转矩形
}PositionPattern;

typedef struct
{
	Point2f coordinate;
}QRCodeCorner;

typedef struct
{
	vector<QRCodeCorner> corner;
}QRCode;


Mat GetRegularROI(Mat &img, RotatedRect &rect);

bool JudgeCornerByX(Mat &img);
bool JudgeCornerByY(Mat &img);

void getQRCode(vector<PositionPattern> &posiPatterns);

double calcDistant(Point2f pointA, Point2f pointB);

double calcCrossProduct(Point2f vectorA, Point2f vectorB);

Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT);

double distPointToLine(Point2f point, Point2f linePointA, Point2f linePointB);

Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB);

#endif //!QR_LOCATE_H