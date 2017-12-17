#include "locator.h"
#include "SerialPort.h"

CSerialPort mySerialPort;

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	markerSet.erase(markerSet.begin(), markerSet.end());
}

Mat Locator::locate(Mat &img)
{
	//变量 & 对象定义及初始化
	markerSet.erase(markerSet.begin(), markerSet.end());
	srcImage = img.clone();
	//debugImage = img.clone();

	//图像初步处理
	cvtColor(srcImage, processImage, COLOR_BGR2GRAY);
	//medianBlur(processImage, processImage, 3);
	//Canny(processImage, processImage, 100, 200, 3);
	threshold(processImage, processImage, 0, 255, THRESH_BINARY | THRESH_OTSU);\
    debugImage = processImage.clone();
	//adaptiveThreshold(processImage, processImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 41, 0);
	imshow("After Process", processImage);

	//寻找轮廓，并存储轮廓的层次信息
	findContours(processImage.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

	//遍历轮廓
	standbyMarkerCnt = 0;

	for (size_t i = 0; i < contours.size(); i++)
	{
		//剔除轮廓面积过小的轮廓
		float area = static_cast<float>(contourArea(contours[i]));
		if (area < 100) { continue; }

		//剔除矩形长宽比不符以及边长过长的轮廓
		RotatedRect rect = minAreaRect(contours[i]);
		float rectWidth = rect.size.width;
		float rectHeight = rect.size.height;
		float ratio = min(rectWidth, rectHeight) / max(rectWidth, rectHeight);

		if (ratio < 0.85) { continue; }
		if (rectWidth > processImage.cols / 4 || rectHeight > processImage.rows / 4) { continue; }

		//统计轮廓层次
		int num = i;
		int layerCounter = 0;

		while (hierarchy[num][2] != -1)
		{
			num = hierarchy[num][2];
			layerCounter = layerCounter + 1;
			if (layerCounter >= 2)
			{
				drawContours(debugImage, contours, static_cast<int>(i), Scalar(255, 255, 255), -1, 8);
				standbyMarker[standbyMarkerCnt++] = rect;

				break;
			}
		}

		//如果预备 Marker 数组填满则跳出
		if (standbyMarkerCnt >= STANDBY_MARKER_SIZE)
		{
			break;
		}
	}

	for (int i = 0; i < standbyMarkerCnt; i++)
	{
		markerSet.push_back(standbyMarker[i]);
	}

	getQRCode(markerSet);
	imshow("Debug", debugImage);

	return QRCodeROI;
}

//从三个marker的信息中获取QRCode
void Locator::getQRCode(vector<Marker> &markerSet)
{
	if (markerSet.size() == (size_t)3)
	{
		//三个 Marker 中心点两两计算距离
		float distanceAB = calcDistance(markerSet[0].center, markerSet[1].center);
		float distanceBC = calcDistance(markerSet[1].center, markerSet[2].center);
		float distanceAC = calcDistance(markerSet[0].center, markerSet[2].center);

		Marker tempMarkerA;
		Marker tempMarkerB;

		Point2f vectorA;
		Point2f vectorB;

		//QRCode 四个Marker中心点
		Point2f pointLeftTop;
		Point2f pointLeftBottom;
		Point2f pointRightTop;
		Point2f pointVirtual;

		//临时存储 Marker 的四个角
		Point2f markerCorners[4];

		//仿射变换的源顶点和目标顶点
		Point2f srcCorner[3];
		Point2f dstCorner[3];

		//找出左上顶点的 Marker
		if (distanceAB > distanceBC && distanceAB > distanceAC)
		{
			markerLeftTop = markerSet[2];

			tempMarkerA = markerSet[0];
			tempMarkerB = markerSet[1];
		}
		else if (distanceBC > distanceAB && distanceBC > distanceAC)
		{
			markerLeftTop = markerSet[0];

			tempMarkerA = markerSet[1];
			tempMarkerB = markerSet[2];
		}
		else if (distanceAC > distanceAB && distanceAC > distanceBC)
		{
			markerLeftTop = markerSet[1];

			tempMarkerA = markerSet[0];
			tempMarkerB = markerSet[2];
		}
		else {}

		//找出左下和右上的 Marker
		vectorA.x = tempMarkerA.center.x - markerLeftTop.center.x;
		vectorA.y = tempMarkerA.center.y - markerLeftTop.center.y;
		vectorB.x = tempMarkerB.center.x - markerLeftTop.center.x;
		vectorB.y = tempMarkerB.center.y - markerLeftTop.center.y;

		if (calcCrossProduct(vectorA, vectorB) > 0)
		{
			markerLeftBottom = tempMarkerB;
			markerRightTop = tempMarkerA;
		}
		else
		{
			markerLeftBottom = tempMarkerA;
			markerRightTop = tempMarkerB;
		}

		//找出四个 Marker 中心点
		pointLeftTop = markerLeftTop.center;
		pointLeftBottom = markerLeftBottom.center;
		pointRightTop = markerRightTop.center;
		pointVirtual = findPointVirtual(pointLeftTop, pointLeftBottom, pointRightTop);

		//找出二维码的三个角
		markerLeftTop.points(markerCorners);
		cornerLeftTop = findFarthestPoint(markerCorners, pointLeftBottom, pointRightTop);

		markerLeftBottom.points(markerCorners);
		cornerLeftBottom = findFarthestPoint(markerCorners, pointLeftTop, pointVirtual);

		markerRightTop.points(markerCorners);
		cornerRightTop = findFarthestPoint(markerCorners, pointLeftTop, pointVirtual);

		//debug
		circle(debugImage, cornerLeftTop,    3, Scalar(255, 0, 0), 2);
		circle(debugImage, cornerLeftBottom, 3, Scalar(0, 255, 0), 2);
		circle(debugImage, cornerRightTop,   3, Scalar(0, 0, 255), 2);

		//为仿射变换的源顶点和目标顶点赋值
		srcCorner[0] = cornerLeftTop;
		srcCorner[1] = cornerLeftBottom;
		srcCorner[2] = cornerRightTop;

		dstCorner[0] = Point(0, 0);
		dstCorner[1] = Point(0, 99);
		dstCorner[2] = Point(99, 0);

		//仿射变换
		Mat affineMatrix;
		affineMatrix = getAffineTransform(srcCorner, dstCorner);
		warpAffine(debugImage, QRCodeROI, affineMatrix, Size(100, 100));
	}
	else
	{
		QRCodeROI = Mat(100, 100, CV_8UC1, Scalar(0));
	}
}

//寻找右下角的虚拟 Marker 中心点
Point2f Locator::findPointVirtual(Point2f pointLT, Point2f pointLB, Point2f pointRT)
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

//寻找一个点集中距离一条直线最远的点
Point2f Locator::findFarthestPoint(Point2f* pointSet, Point2f linePointA, Point2f linePointB)
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
//提取处理后图片的信号量
void Locator::ExtractingInformation(Mat img)
{
	//轮廓定义
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(img, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());

	RotatedRect Rect[4];
	size_t k = 0;
	int leftCount = 0;
	int rightCount = 0;
	int upCount = 0;
	int downCount = 0;

	for (size_t i = 0; i < contours.size(); i++)
	{
		//剔除轮廓面积过小的轮廓
		float area = static_cast<float>(contourArea(contours[i]));

		if (area < 10 || area >700) { continue; }

		//剔除矩形长宽比不符以及边长过长的轮廓
		RotatedRect rect = minAreaRect(contours[i]);

		float rectWidth = rect.size.width;
		float rectHeight = rect.size.height;
		float ratio = min(rectWidth, rectHeight) / max(rectWidth, rectHeight);
		if (ratio < 0.85) { continue; }
		if (rectWidth > processImage.cols / 4 || rectHeight > processImage.rows / 4) { continue; }
		Rect[k] = rect;
		//cout << Rect[k].center << endl;

		if (Rect[k].center.x - 50 > 10) {
			rightCount++;
		}
		else if (Rect[k].center.x - 50 < -10) {
			leftCount++;
		}
		else if (Rect[k].center.y - 50 > 10) {
			downCount++;
		}
		else if (Rect[k].center.y - 50 < -10) {
			upCount++;
		}
		k++;
	}
	if (rightCount == 0 || leftCount == 0 || downCount == 0 || upCount == 0) {
		SendMessageToUSB(NOT_HIT);
	}
	else if (rightCount == 1 || leftCount == 1 || downCount == 1 || upCount == 1) {
		SendMessageToUSB(HIT);
	}
	else if (rightCount == 0 || leftCount == 1 || downCount == 0 || upCount == 1) {
		SendMessageToUSB(LEFT_UP);
	}
	else if (rightCount == 0 || leftCount == 1 || downCount == 1 || upCount == 0) {
		SendMessageToUSB(LEFT_DOWN);
	}
	else if (rightCount == 1 || leftCount == 0 || downCount == 0 || upCount == 1) {
		SendMessageToUSB(RIGHT_UP);
	}
	else if (rightCount == 1 || leftCount == 0 || downCount == 1 || upCount == 0) {
		SendMessageToUSB(RIGHT_DOWN);
	}
	else if (rightCount == 1 || leftCount == 1 || downCount == 1 || upCount == 0) {
		SendMessageToUSB(FORWARD);
	}
	else if (rightCount == 1 || leftCount == 1 || downCount == 0 || upCount == 1) {
		SendMessageToUSB(BACK);
	}
}

int SerialPortInit(void) {
	#define SERIAL_PORT_INIT_SUCCESS 1
	#define SERIAL_PORT_INIT_FAULT 0
	if (!mySerialPort.InitPort(9, CBR_115200))
	{
		std::cout << "initPort fail !" << std::endl;
		return SERIAL_PORT_INIT_FAULT;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}

	if (!mySerialPort.OpenListenThread())
	{
		std::cout << "OpenListenThread fail !" << std::endl;
		return SERIAL_PORT_INIT_FAULT;
	}
	else
	{
		std::cout << "OpenListenThread success !" << std::endl;
	}
		return SERIAL_PORT_INIT_SUCCESS;
}
//串口发数
void SendMessageToUSB(unsigned char getMessage) {
	if (mySerialPort.WriteData(&getMessage, 1) == false) {
		std::cout << "fault" << std::endl;
	}
	else {
		std::cout << "ok" << std::endl;
	}
}
