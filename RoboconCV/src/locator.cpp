#include "locator.h"
#include "modeConfig.h"

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	markerPair.erase(markerPair.begin(), markerPair.end());
}

Signal Locator::locate(Mat &img)
{
	//变量 & 对象定义及初始化
	markerPair.erase(markerPair.begin(), markerPair.end());
	srcImage = img.clone();
	debugImage = img.clone();

	//图像预处理
	cvtColor(srcImage, preProcImage, COLOR_BGR2GRAY);
	adaptiveThreshold(preProcImage, preProcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 25, 0);

#ifdef IMSHOW_DEBUG_IMAGE
	imshow("After Process", preProcImage);
#endif //IMSHOW_DEBUG_IMAGE

	dstSignal.image = Mat(REGULAR_SIGNAL_HEIGHT, REGULAR_SIGNAL_WIDTH, CV_8UC3, Scalar(0, 0, 0));
	dstSignal.lable = false;

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
		int num = static_cast<int>(i);
		int layerCounter = 0;

		while (hierarchy[num][2] != -1)
		{
			num = hierarchy[num][2];
			layerCounter++;

			if (layerCounter == 1)
			{
				//检查此层级轮廓数量是否 == 1
				if (hierarchy[num][0] != -1) { continue; }
			}

			if (layerCounter == 2 && hierarchy[num][2] == -1)
			{
				//检查此层级轮廓数量是否 == 2
				if (hierarchy[num][0] == -1) { continue; }
				
				num = hierarchy[num][0];
				if (hierarchy[num][0] != -1) { continue; }

				//多边形拟合找出四边形的轮廓
				vector<Point> vertex;
				approxPolyDP(contours[i], vertex, 4, true);
				if (vertex.size() != 4) { continue; }

				drawContours(debugImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);

				//根据Marker的重心判断上下左右四个定点，并按顺序存入Possible Marker
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

		//如果用于存储Possible Marker的数组填满则跳出
		if (psbMarkerCnt >= POSSIBLE_MARKER_NUM)
		{
			break;
		}
	}

	markerPair = findMarkerPair(psbMarker, psbMarkerCnt);

	dstSignal = getSignal(markerPair);

#ifdef IMSHOW_DEBUG_IMAGE
	imshow("Debug", debugImage);
#endif //IMSHOW_DEBUG_IMAGE

	return dstSignal;
}

//从可能的 Marker 里面找出最终的 Marker 对
vector<Marker> Locator::findMarkerPair(Marker* psbMarker, int MarkerCnt)
{
	vector<Marker> markerPair;

	//用于判断Marker顶边四点共线
	Point2f vectorA;
	Point2f vectorB;
	Point2f vectorC;
	float sharpAngleA;
	float sharpAngleB;

	//用于寻找两个Marker距离最短的两个点对，然后找到同一Marker中两点中点的连线
	Point markerAPoint1;
	Point markerBPoint1;
	Point markerAPoint2;
	Point markerBPoint2;
	Point markerAPointMid;
	Point markerBPointMid;
	float crtDistance;
	float minDistanceA = 9999;
	float minDistanceB = 9999;

	//遍历任意两个Marker
	for (int i = 0; i < MarkerCnt; i++)
	{
		for (int j = i + 1; j < MarkerCnt; j++)
		{
			//检查两个Marker的顶边是否在同一条直线上（四点共线）
			vectorA.x = static_cast<float>(psbMarker[i].cornerRightTop.x - psbMarker[i].cornerLeftTop.x);
			vectorA.y = static_cast<float>(psbMarker[i].cornerRightTop.y - psbMarker[i].cornerLeftTop.y);

			vectorB.x = static_cast<float>(psbMarker[j].cornerRightTop.x - psbMarker[j].cornerLeftTop.x);
			vectorB.y = static_cast<float>(psbMarker[j].cornerRightTop.y - psbMarker[j].cornerLeftTop.y);

			vectorC.x = static_cast<float>(psbMarker[j].cornerLeftTop.x - psbMarker[i].cornerLeftTop.x);
			vectorC.y = static_cast<float>(psbMarker[j].cornerLeftTop.y - psbMarker[i].cornerLeftTop.y);

			sharpAngleA = calcSharpAngle(vectorA, vectorC);
			sharpAngleB = calcSharpAngle(vectorB, vectorC);

			if (sharpAngleA >= Pi / 12.0f || sharpAngleA >= Pi / 12.0f)
			{
				continue;
			}

			//求两Marker中距离最短的两个点对
			for (int m = 0; m < 4; m++)
			{
				for (int n = 0; n < 4; n++)
				{
					crtDistance = calcDistance(psbMarker[i][m], psbMarker[j][n]);
					if (crtDistance < minDistanceB)
					{
						if (crtDistance < minDistanceA)
						{
							minDistanceB = minDistanceA;
							minDistanceA = crtDistance;

							markerBPoint1 = markerAPoint1;
							markerBPoint2 = markerAPoint2;
							markerAPoint1 = psbMarker[i][m];
							markerAPoint2 = psbMarker[j][n];
						}
						else
						{
							minDistanceB = crtDistance;

							markerBPoint1 = psbMarker[i][m];
							markerBPoint2 = psbMarker[j][n];
						}
					}
				}
			}
			markerAPointMid = calcMidpoint(markerAPoint1, markerBPoint1);
			markerBPointMid = calcMidpoint(markerAPoint2, markerBPoint2);

			minDistanceA = 9999;
			minDistanceB = 9999;

			//中点连线的线迭代器，用于遍历像素以找出线段上的像素关系
			LineIterator timingPatternIt(preProcImage, markerAPointMid, markerBPointMid);
			uchar crtPixel = 255;
			vector<int> timingCounter;
			int crtTimingCounter = 0;
			int timingBuffer = 0;

			//用线迭代器进行迭代，将每一像素段的像素数量进行存储
			for (int x = 0; x < timingPatternIt.count; x++)
			{
				if (abs((uchar)**timingPatternIt - crtPixel) <= 127)
				{
					crtTimingCounter++;
					timingBuffer = 0;
				}
				else
				{
					crtTimingCounter++;
					timingBuffer++;

					if (timingBuffer >= 3)
					{
						timingCounter.push_back(crtTimingCounter - timingBuffer);
						crtTimingCounter = timingBuffer;
						crtPixel = abs(crtPixel - 255);
						timingBuffer = 0;
					}
				}

				timingPatternIt++;
			}
			timingCounter.push_back(crtTimingCounter - timingBuffer);

			//检查 0 1 3 4 号像素块的比例是否符合1：1：1：1
			if (timingCounter.size() == 5)
			{
				/*for (uint p = 0; p < timingCounter.size(); p++)
				{
					cout << timingCounter[p] << "  ";
				}
				cout << endl;*/
				
				int ratio[5];

				for (int x = 0; x < 5; x++)
				{
					ratio[x] = static_cast<int>((float)timingCounter[x] / ((float)timingPatternIt.count / 165.0f * 35.0f) + 0.5f);
				}

				if (ratio[0] == 1 && ratio[1] == 1 && ratio[3] == 1 && ratio[4] == 1)
				{
					cv::line(debugImage, markerAPointMid, markerBPointMid, Scalar(0, 255, 0), 2);

					markerPair.push_back(psbMarker[i]);
					markerPair.push_back(psbMarker[j]);

					return markerPair;
				}
			}
		}
	}
	return markerPair;
}

//从两个Marker的信息中获取Signal
Signal Locator::getSignal(vector<Marker> markerPair)
{
	//要找的目标Signal
	Signal crtSignal;
	crtSignal.image = Mat(REGULAR_SIGNAL_HEIGHT, REGULAR_SIGNAL_WIDTH, CV_8UC3, Scalar(0, 0, 0));
	crtSignal.lable = false;

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
		srcCorner[0] = markerLeft.cornerRightTop;
		srcCorner[1] = markerLeft.cornerRightBottom;
		srcCorner[2] = markerRight.cornerLeftTop;
		srcCorner[3] = markerRight.cornerLeftBottom;

		dstCorner[0] = Point(0, 0);
		dstCorner[1] = Point(0, REGULAR_SIGNAL_HEIGHT);
		dstCorner[2] = Point(REGULAR_SIGNAL_WIDTH, 0);
		dstCorner[3] = Point(REGULAR_SIGNAL_WIDTH, REGULAR_SIGNAL_HEIGHT);

		cv::Mat transMatrix = cv::getPerspectiveTransform(srcCorner, dstCorner);
		cv::warpPerspective(srcImage, crtSignal.image, transMatrix, crtSignal.image.size());

		crtSignal.lable = true;
	}

	return crtSignal;
}