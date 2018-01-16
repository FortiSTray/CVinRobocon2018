#include "locator.h"

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	markerPair.erase(markerPair.begin(), markerPair.end());
}

Signal Locator::locate(Mat &img)
{
	char fileName[32];
	static int fileSerial = 0;

	//变量 & 对象定义及初始化
	markerPair.erase(markerPair.begin(), markerPair.end());
	srcImage = img.clone();
	debugImage = img.clone();

	//图像预处理
	cvtColor(srcImage, preProcImage, COLOR_BGR2GRAY);
	//equalizeHist(preProcImage, preProcImage);
	//medianBlur(preProcImage, preProcImage, 3);
	//imshow("equa", preProcImage);
	//Canny(preProcImage, preProcImage, 100, 200, 3);
	//threshold(preProcImage, preProcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	adaptiveThreshold(preProcImage, preProcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 15, 0);

	imshow("After Process", preProcImage);

	dstSignal.image = Mat(REGULAR_SIGNAL_HEIGHT, REGULAR_SIGNAL_WIDTH, CV_8UC3, Scalar(0, 0, 0));
	dstSignal.lable = 0;
	//return dstSignal;

	//寻找轮廓，并存储轮廓的层次信息
	findContours(preProcImage.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

	//遍历轮廓
	psbMarkerCnt = 0;

	for (size_t i = 0; i < contours.size(); i++)
	{
		//剔除轮廓面积过小的轮廓
		float area = static_cast<float>(contourArea(contours[i]));
		if (area < 100) { continue; }

		//统计轮廓层次
		int num = i;
		int layerCounter = 0;

		while (hierarchy[num][2] != -1)
		{
			num = hierarchy[num][2];
			layerCounter++;

			if (layerCounter == /*5*/2)
			{
				//检查此层级轮廓数量是否 >= 2
				if (hierarchy[num][0] == -1) { continue; }

				//多边形拟合找出四边形的轮廓
				vector<Point> vertex;
				approxPolyDP(contours[i], vertex, 5, true);
				if (vertex.size() != 4) { continue; }

				drawContours(debugImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);

				Point crtGravityCenter = calcGravityCenter(vertex);
				for (auto crtVertex : vertex)
				{
					if (crtVertex.x < crtGravityCenter.x && crtVertex.y < crtGravityCenter.y)
					{
						psbMarker[psbMarkerCnt].cornerLeftTop = crtVertex;
					}
					else if (crtVertex.x < crtGravityCenter.x && crtVertex.y >= crtGravityCenter.y)
					{
						psbMarker[psbMarkerCnt].cornerLeftBottom = crtVertex;
					}
					else if (crtVertex.x >= crtGravityCenter.x && crtVertex.y < crtGravityCenter.y)
					{
						psbMarker[psbMarkerCnt].cornerRightTop = crtVertex;
					}
					else if (crtVertex.x >= crtGravityCenter.x && crtVertex.y >= crtGravityCenter.y)
					{
						psbMarker[psbMarkerCnt].cornerRightBottom = crtVertex;
					}
				}

				cv::circle(debugImage, psbMarker[psbMarkerCnt].cornerLeftTop, 2, cv::Scalar(0, 255, 0), 2);
				cv::circle(debugImage, psbMarker[psbMarkerCnt].cornerLeftBottom, 2, cv::Scalar(0, 0, 255), 2);
				cv::circle(debugImage, psbMarker[psbMarkerCnt].cornerRightTop, 2, cv::Scalar(255, 0, 255), 2);
				cv::circle(debugImage, psbMarker[psbMarkerCnt].cornerRightBottom, 2, cv::Scalar(255, 255, 0), 2);

				psbMarkerCnt++;

				break;
			}
		}

		//如果预备 Marker 数组填满则跳出
		if (psbMarkerCnt >= POSSIBLE_MARKER_NUM)
		{
			break;
		}
	}

	if (psbMarkerCnt >= 2)
	{
		markerPair = findMarkerPair(psbMarker, psbMarkerCnt);
	}

	dstSignal = getSignal(markerPair);

	imshow("Debug", debugImage);

	return dstSignal;
}

//从可能的 Marker 里面找出最终的 Marker 对
vector<Marker> Locator::findMarkerPair(Marker* psbMarker, int MarkerCnt)
{
	vector<Marker> markerPair;
	markerPair.erase(markerPair.begin(), markerPair.end());

	if (MarkerCnt < 2)
	{
		return markerPair;
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			markerPair.push_back(psbMarker[i]);
		}
	}

	return markerPair;
}

//从三个marker的信息中获取Signal
Signal Locator::getSignal(vector<Marker> markerPair)
{
	//要找的目标Signal
	Signal crtSignal;
	crtSignal.image = Mat(REGULAR_SIGNAL_HEIGHT, REGULAR_SIGNAL_WIDTH, CV_8UC3, Scalar(0, 0, 0));
	crtSignal.lable = 0;

	//左Marker和右Marker
	Marker markerLeft;
	Marker markerRight;

	//透视变换的源顶点和目标顶点
	Point2f srcCorner[4];
	Point2f dstCorner[4];

	if (markerPair.size() != 2)
	{
		return crtSignal;
	}
	else
	{
		Point tempMidpoint = calcMidpoint(markerPair[0].cornerLeftTop, markerPair[1].cornerLeftTop);
		
		if (markerPair[0].cornerLeftTop.x < tempMidpoint.x)
		{
			markerLeft  = markerPair[0];
			markerRight = markerPair[1];
		}
		else
		{
			markerRight = markerPair[0];
			markerLeft  = markerPair[1];
		}

		//透视变换
		srcCorner[0] = markerLeft.cornerLeftTop;
		srcCorner[1] = markerLeft.cornerLeftBottom;
		srcCorner[2] = markerRight.cornerRightTop;
		srcCorner[3] = markerRight.cornerRightBottom;

		dstCorner[0] = Point(0, 0);
		dstCorner[1] = Point(0, REGULAR_SIGNAL_HEIGHT);
		dstCorner[2] = Point(REGULAR_SIGNAL_WIDTH, 0);
		dstCorner[3] = Point(REGULAR_SIGNAL_WIDTH, REGULAR_SIGNAL_HEIGHT);

		cv::Mat transMatrix = cv::getPerspectiveTransform(srcCorner, dstCorner);
		cv::warpPerspective(srcImage, crtSignal.image, transMatrix, crtSignal.image.size());

		crtSignal.lable = 1;
	}

	return crtSignal;
}

