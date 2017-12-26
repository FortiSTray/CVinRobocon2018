#include "locator.h"

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	markerSet.erase(markerSet.begin(), markerSet.end());
}

QRCode Locator::locate(Mat &img)
{
	//变量 & 对象定义及初始化
	markerSet.erase(markerSet.begin(), markerSet.end());
	srcImage = img.clone();
	debugImage = img.clone();

	//图像预处理
	cvtColor(srcImage, preProcImage, COLOR_BGR2GRAY);
	//Canny(preProcImage, testImage, 100, 200, 3);
	//imshow("Canny1", testImage);
	//equalizeHist(preProcImage, preProcImage);
	//medianBlur(preProcImage, preProcImage, 3);
	//imshow("equa", preProcImage);
	Canny(preProcImage, preProcImage, 100, 200, 3);
	//threshold(preProcImage, preProcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//adaptiveThreshold(preProcImage, preProcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 55, 0);
	imshow("After Process", preProcImage);

	//寻找轮廓，并存储轮廓的层次信息
	findContours(preProcImage.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

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
		if (rectWidth > preProcImage.cols / 4 || rectHeight > preProcImage.rows / 4) { continue; }

		//统计轮廓层次
		int num = i;
		int layerCounter = 0;

		while (hierarchy[num][2] != -1)
		{
			num = hierarchy[num][2];
			layerCounter = layerCounter + 1;
			if (layerCounter >= 5)
			{
				drawContours(debugImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);
				standbyMarker[standbyMarkerCnt++] = rect;
				cout << standbyMarkerCnt << "  " << rect.size.width << "  " << rect.angle  << endl;

				break;
			}
		}

		//如果预备 Marker 数组填满则跳出
		if (standbyMarkerCnt >= STANDBY_MARKER_NUM)
		{
			break;
		}
	}

	if (standbyMarkerCnt >= 3)
	{
		markerSet = findMarkerReal(standbyMarker, standbyMarkerCnt);
	}

	getQRCode(markerSet);
	imshow("Debug", debugImage);

	return dstQRCode;
}

//从三个marker的信息中获取QRCode
void Locator::getQRCode(vector<Marker> &mkrSet)
{
	if (mkrSet.size() == (size_t)3)
	{
		//三个 Marker 中心点两两计算距离
		float distanceAB = calcDistance(mkrSet[0].center, mkrSet[1].center);
		float distanceBC = calcDistance(mkrSet[1].center, mkrSet[2].center);
		float distanceAC = calcDistance(mkrSet[0].center, mkrSet[2].center);

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
			markerLeftTop = mkrSet[2];

			tempMarkerA = mkrSet[0];
			tempMarkerB = mkrSet[1];
		}
		else if (distanceBC > distanceAB && distanceBC > distanceAC)
		{
			markerLeftTop = mkrSet[0];

			tempMarkerA = mkrSet[1];
			tempMarkerB = mkrSet[2];
		}
		else if (distanceAC > distanceAB && distanceAC > distanceBC)
		{
			markerLeftTop = mkrSet[1];

			tempMarkerA = mkrSet[0];
			tempMarkerB = mkrSet[2];
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
		dstCorner[1] = Point(0, REGULAR_QRCODE_SIDE - 1);
		dstCorner[2] = Point(REGULAR_QRCODE_SIDE - 1, 0);

		//仿射变换
		Mat affineMatrix;
		affineMatrix = getAffineTransform(srcCorner, dstCorner);
		warpAffine(srcImage, dstQRCode.image, affineMatrix, Size(REGULAR_QRCODE_SIDE, REGULAR_QRCODE_SIDE));

		//QRCode的lable赋值为1表示找到QRCode
		dstQRCode.lable = 1;
	}
	else
	{
		dstQRCode.image = Mat(REGULAR_QRCODE_SIDE, REGULAR_QRCODE_SIDE, CV_8UC3, Scalar(0, 0, 0));
		dstQRCode.lable = 0;
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

//从预备 Marker 的集合里面找出 真·Marker
vector<Marker> Locator::findMarkerReal(Marker* standbyMarker, int MarkerCnt)
{
	Marker tempMarker;
	
	int serialNumByWidth[10];
	int widthCnt = 0;

	int serialNumByAngle[10];
	int angleCnt = 0;
	bool angleScanDoneFlag = 0;
	
	vector<Marker> markerReal;

	//根据 width 判断 Marker
	for (int i = 0; i < MarkerCnt; i++)
	{
		for (int j = i + 1; j < MarkerCnt; j++)
		{
			if (standbyMarker[i].size.width > standbyMarker[j].size.width)
			{
				tempMarker       = standbyMarker[i];
				standbyMarker[i] = standbyMarker[j];
				standbyMarker[j] = tempMarker      ;
			}
		}
	}

	for (int i = 0; i < MarkerCnt - 1; i++)
	{
		widthCnt = 0;

		while (standbyMarker[i + 1].size.width - standbyMarker[i].size.width < 5.0f)
		{
			serialNumByWidth[widthCnt++] = i++;
			if (i == MarkerCnt - 1) { break; }
		}
		serialNumByWidth[widthCnt++] = i;

		if (widthCnt >= 3) { break; }
	}

	if (widthCnt < 3)
	{
		markerReal.erase(markerReal.begin(), markerReal.end());
		return markerReal;
	}

	//跟据 angle 判断 Marker
	for (int i = 0; i < MarkerCnt; i++)
	{
		for (int j = i + 1; j < MarkerCnt; j++)
		{
			if (standbyMarker[i].angle > standbyMarker[j].angle)
			{
				tempMarker       = standbyMarker[i];
				standbyMarker[i] = standbyMarker[j];
				standbyMarker[j] = tempMarker      ;
			}
		}
	}

	for (int i = 0; i < MarkerCnt - 1; i++)
	{
		angleCnt = 0;
		
		while (standbyMarker[i + 1].angle - standbyMarker[i].angle < 5.0f)
		{
			serialNumByAngle[angleCnt++] = i++;
			if (angleCnt == MarkerCnt - 1) { break; }

			if (i == MarkerCnt - 1)
			{
				angleScanDoneFlag = 1;
				if (standbyMarker[0].angle - standbyMarker[MarkerCnt - 1].angle + 90.0f < 5.0f)
				{
					serialNumByAngle[angleCnt++] = MarkerCnt - 1;
					i = 0;
					if (angleCnt == MarkerCnt - 1) { break; }
				}
			}
		}
		serialNumByAngle[angleCnt++] = i;

		if (angleCnt >= 3 || angleScanDoneFlag == 1) { break; }
	}

	if (angleCnt < 3)
	{
		markerReal.erase(markerReal.begin(), markerReal.end());
		return markerReal;
	}

	//寻找 width 和 angle 的匹配
	for (int i = 0; i < widthCnt; i++)
	{
		for (int j = 0; j < angleCnt; j++)
		{
			if (serialNumByWidth[i] == serialNumByAngle[j])
			{
				markerReal.push_back(standbyMarker[serialNumByWidth[i]]);
				if (markerReal.size() == 3) { return markerReal; }
			}
		}
	}

	markerReal.erase(markerReal.begin(), markerReal.end());
	return markerReal;
}
