#include "locator.h"

Locator::Locator(void)
{
	
}
Locator::~Locator(void)
{
	markerSet.erase(markerSet.begin(), markerSet.end());
}

Mat Locator::locate(Mat &img)
{
	Marker tmpMarker;
	markerSet.erase(markerSet.begin(), markerSet.end());

	srcImage = img.clone();
	markerImage = img.clone();

	namedWindow("Src", CV_WINDOW_AUTOSIZE);
	imshow("Src", srcImage);

	cvtColor(srcImage, srcImage, COLOR_BGR2GRAY);
	//medianBlur(srcImage, srcImage, 3);
	//imshow("blur", srcImage);
	//threshold(srcImage, srcImage, 0, 255, THRESH_BINARY | THRESH_OTSU);
	adaptiveThreshold(srcImage, srcImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 55, 0);
	imshow("Threshold", srcImage);

	//detect rectangle now
	findContours(srcImage.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());

	for (size_t i = 0; i < contours.size(); i++)
	{
		float area = static_cast<float>(contourArea(contours[i]));
		if (area < 100) continue;
		RotatedRect rect = minAreaRect(contours[i]);

		//根据矩形特征进行几何分析
		float rectWidth = rect.size.width;
		float rectHeight = rect.size.height;
		float ratio = min(rectWidth, rectHeight) / max(rectWidth, rectHeight);

		if (ratio > 0.85 && rectWidth < srcImage.cols / 4 && rectHeight < srcImage.rows / 4)
		{
			int num = i;
			int layerCounter = 0;

			while (hierarchy[num][2] != -1)
			{
				num = hierarchy[num][2];
				layerCounter = layerCounter + 1;
				if (layerCounter >= 2)
				{
					drawContours(markerImage, contours, static_cast<int>(i), Scalar(255, 0, 0), 2, 8);
					tmpMarker.contour = contours[i];
					tmpMarker.outerRect = rect;
					markerSet.push_back(tmpMarker);

					break;
				}
			}
		}
	}

	getQRCode(markerSet);

	return QRCodeROI;
}

void Locator::getQRCode(vector<Marker> &markerSet)
{
	if (markerSet.size() == (size_t)3)
	{
		Point2f pointA = markerSet[0].outerRect.center;
		Point2f pointB = markerSet[1].outerRect.center;
		Point2f pointC = markerSet[2].outerRect.center;

		float distantAB = calcDistant(pointA, pointB);
		float distantBC = calcDistant(pointB, pointC);
		float distantAC = calcDistant(pointA, pointC);

		Point2f pointLeftTop;
		Point2f pointLeftBottom;
		Point2f pointRightTop;
		Point2f pointVirtual;

		RotatedRect rectLeftTop;
		RotatedRect rectLeftBottom;
		RotatedRect rectRightTop;

		Point2f pointTemp;

		Point2f vectorA;
		Point2f vectorB;

		Point2f tempCorners[4];

		Point2f cornerLeftTop;
		Point2f cornerLeftBottom;
		Point2f cornerRightTop;

		Point2f srcCorner[3];
		Point2f dstCorner[3];

		rectLeftTop = markerSet[0].outerRect;

		//找出三个点
		if (distantAB > distantBC && distantAB > distantAC)
		{
			pointLeftTop = pointC;
			rectLeftTop = markerSet[2].outerRect;

			vectorA.x = pointA.x - pointLeftTop.x;
			vectorA.y = pointA.y - pointLeftTop.y;
			vectorB.x = pointB.x - pointLeftTop.x;
			vectorB.y = pointB.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointB;
				pointRightTop = pointA;
				rectLeftBottom = markerSet[1].outerRect;
				rectRightTop = markerSet[0].outerRect;
			}
			else
			{
				pointLeftBottom = pointA;
				pointRightTop = pointB;
				rectLeftBottom = markerSet[0].outerRect;
				rectRightTop = markerSet[1].outerRect;
			}
		}
		else if (distantBC > distantAB && distantBC > distantAC)
		{
			pointLeftTop = pointA;
			rectLeftTop = markerSet[0].outerRect;

			vectorA.x = pointB.x - pointLeftTop.x;
			vectorA.y = pointB.y - pointLeftTop.y;
			vectorB.x = pointC.x - pointLeftTop.x;
			vectorB.y = pointC.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointC;
				pointRightTop = pointB;
				rectLeftBottom = markerSet[2].outerRect;
				rectRightTop = markerSet[1].outerRect;
			}
			else
			{
				pointLeftBottom = pointB;
				pointRightTop = pointC;
				rectLeftBottom = markerSet[1].outerRect;
				rectRightTop = markerSet[2].outerRect;
			}
		}
		else if (distantAC > distantAB && distantAC > distantBC)
		{
			pointLeftTop = pointB;
			rectLeftTop = markerSet[1].outerRect;

			vectorA.x = pointA.x - pointLeftTop.x;
			vectorA.y = pointA.y - pointLeftTop.y;
			vectorB.x = pointC.x - pointLeftTop.x;
			vectorB.y = pointC.y - pointLeftTop.y;

			if (calcCrossProduct(vectorA, vectorB) > 0)
			{
				pointLeftBottom = pointC;
				pointRightTop = pointA;
				rectLeftBottom = markerSet[2].outerRect;
				rectRightTop = markerSet[0].outerRect;
			}
			else
			{
				pointLeftBottom = pointA;
				pointRightTop = pointC;
				rectLeftBottom = markerSet[0].outerRect;
				rectRightTop = markerSet[2].outerRect;
			}
		}
		else {}

		//找出右下虚拟点
		pointVirtual = findPointVirtual(pointLeftTop, pointLeftBottom, pointRightTop);

		//找出二维码的三个角
		rectLeftTop.points(tempCorners);
		cornerLeftTop = findFarthestPoint(tempCorners, pointLeftBottom, pointRightTop);

		rectLeftBottom.points(tempCorners);
		cornerLeftBottom = findFarthestPoint(tempCorners, pointLeftTop, pointVirtual);

		rectRightTop.points(tempCorners);
		cornerRightTop = findFarthestPoint(tempCorners, pointLeftTop, pointVirtual);

		Mat temp = srcImage.clone();
		Mat output;
		circle(temp, cornerLeftTop, 3, Scalar(255, 255, 0));
		circle(temp, cornerLeftBottom, 3, Scalar(0, 255, 0));
		circle(temp, cornerRightTop, 3, Scalar(0, 0, 255));
		imshow("point", temp);

		srcCorner[0] = cornerLeftTop;
		srcCorner[1] = cornerLeftBottom;
		srcCorner[2] = cornerRightTop;

		dstCorner[0] = Point(0, 0);
		dstCorner[1] = Point(0, 50);
		dstCorner[2] = Point(50, 0);

		Mat affineMatrix;
		affineMatrix = getAffineTransform(srcCorner, dstCorner);
		warpAffine(temp, output, affineMatrix, Size(50, 50));
		imshow("QR", output);
	}
}

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