#ifndef _LOCATOR_H
#define _LOCATOR_H

#include <iostream>
#include "opencv2/opencv.hpp"
#include "SerialPort.h"
#include <Windows.h>
#include <iostream> 
#include <fstream> 
#include <process.h>
#include "SerialPort.h"
#include <fstream>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

#define REGULAR_CORNER_SIDE 70
#define REGULAR_QRCODE_SIDE 28
#define STANDBY_MARKER_SIZE 10

#define HIT (unsigned char)'1'
#define NOT_HIT  (unsigned char)'2'
#define LEFT_UP  (unsigned char)'3'
#define LEFT_DOWN  (unsigned char)'4'
#define RIGHT_UP  (unsigned char)'5'
#define RIGHT_DOWN  (unsigned char)'6'
#define FORWARD  (unsigned char)'7'
#define BACK  (unsigned char)'8'

typedef RotatedRect Marker;
//���ڳ�ʼ��
int SerialPortInit(void);
//��Ϣ���ͺ���
void SendMessageToUSB(unsigned char getMessage);


class Locator
{
public:
	Locator(void);
	~Locator(void);

	//QRCode��λ������
	Mat locate(Mat &img);

	//��ȡ�ź���
	void ExtractingInformation(Mat img);

private:
	//������marker����Ϣ�л�ȡQRCode
	void getQRCode(vector<Marker> &markerSet);

	//Ѱ�����½ǵ�����marker���ĵ�
	Point2f findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT);

	//Ѱ��һ���㼯�о���һ��ֱ����Զ�ĵ�
	Point2f findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB);

	//�����������
	inline float calcDistance(Point2f pointA, Point2f pointB)
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

		//����ֱ�ߵ���������
		a = linePointA.y - linePointB.y;
		b = linePointB.x - linePointA.x;
		c = linePointA.x * linePointB.y - linePointB.x * linePointA.y;

		dist = fabsf(a * point.x + b * point.y + c) / sqrtf(a * a + b * b);
		return dist;
	}

private:
	//����ͼ����
	Mat srcImage;
	Mat processImage;
	Mat debugImage;
	Mat QRCodeROI;

	//��������
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	
	//Ԥ�� Marker
	Marker standbyMarker[STANDBY_MARKER_SIZE];
	int standbyMarkerCnt;

	//��λ��Ǽ��϶���
	vector<Marker> markerSet;

	//QRCode Marker
	Marker markerLeftTop;
	Marker markerLeftBottom;
	Marker markerRightTop;

	//QRCode ����
	Point2f cornerLeftTop;
	Point2f cornerLeftBottom;
	Point2f cornerRightTop;
};
#endif